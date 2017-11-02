/*
 * tclgdb_Init and tclgdb_SafeInit
 *
 * Copyright (C) 2016 - 2017 FlightAware
 *
 * Freely redistributable under the Berkeley copyright.  See license.terms
 * for details.
 */

#include <tcl.h>
#include <string.h>
#include <unistd.h>

#include "tclDecls.h"
#include "tclgdbIntDecls.h"

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT

static char buffer[2*1024];
static char cmdbuffer[512];
#ifdef HAVE_TCLINT_H
static char path_buffer[2048];
#endif
static char const_unknown[8];

static char * get_tcl_source_file(Tcl_Interp *);

static int (*tcl_get_frame) (Tcl_Interp *interp, const char *str, TclGdbCallFrame **framePtrPtr) = NULL;

/*
 * Callback in C for Tcl's cmdtrace.
 *
 * In gdb, simply add "break tclgdb_cmdstep".  This will be a deferred load, so
 * answer yes to deferred breakpoint.
 */
void tclgdb_cmdstep(ClientData clientData,
		Tcl_Interp *interp,     /* Current interpreter. */
		int level,              /* Current trace level. */
		char *command,          /* The command being traced (after
								 * substitutions). */
		Tcl_CmdProc *cmdProc,   /* Points to command's command procedure. */
		ClientData cmdClientData,  /* Client data associated with command procedure. */
		int argc,               /* Number of arguments. */
		const char *argv[])     /* Argument strings. */
{
	/* Some code so that gcc does not optimize out this function. */
	char *s = get_tcl_source_file(interp);
	s = (s == NULL ? "unknown" : s);
	strncpy(cmdbuffer, command, sizeof(cmdbuffer) - 1);
	snprintf(buffer, sizeof(buffer) - 1, "%d: @@ %s @@ %s", level, s, cmdbuffer);
	/* write to a bad FD, but we can see it in truss or strace */
	write(-1, buffer, strlen(buffer));
}

typedef struct tclgdb_objectClientData {
	int object_magic;
	int has_trace;
	Tcl_Trace trace;
} tclgdb_objectClientData;

#define GDBTOOLS_OBJECT_MAGIC 74352357

static void tclgdb_CmdDeleteProc(ClientData clientData)
{
	if (clientData != NULL) {
		ckfree(clientData);
	}
}

static int tclgdbObjCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objvp[])
{
	if (clientData != NULL
	    && objc == 2
            && strcmp("off", Tcl_GetString(objvp[1])) == 0) {
		tclgdb_objectClientData *cd = (tclgdb_objectClientData *)clientData;
		if (cd->has_trace) {
			Tcl_DeleteTrace(interp, cd->trace);
			cd->has_trace = 0;
		}
	}

	if (clientData != NULL
	    && (objc == 1 || (objc == 2
		&& strcmp("on", Tcl_GetString(objvp[1])) == 0))) {
		tclgdb_objectClientData *cd = (tclgdb_objectClientData *)clientData;
		if (!cd->has_trace) {
			cd->trace = Tcl_CreateTrace(interp, 50000, tclgdb_cmdstep, &buffer);
			cd->has_trace = 1;
		}
	}

	return TCL_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * tclgdb_Init --
 *
 *	Initialize the tclgdb extension.  The string "tclgdb"
 *      in the function name must match the PACKAGE declaration at the top of
 *	configure.in.
 *
 * Results:
 *	A standard Tcl result
 *
 * Side effects:
 *	One new command "::tclgdb::tclgdb" is added to the Tcl interpreter.
 *
 *----------------------------------------------------------------------
 */

	EXTERN int
Tclgdb_Init(Tcl_Interp *interp)
{
	Tcl_Namespace *namespace;

	if (Tcl_InitStubs(interp, "8.6", 0) == NULL) {
		return TCL_ERROR;
	}

	if (Tcl_PkgRequire(interp, "Tcl", "8.6", 0) == NULL) {
		return TCL_ERROR;
	}

	if (Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_VERSION) != TCL_OK) {
		return TCL_ERROR;
	}

	namespace = Tcl_CreateNamespace (interp, "::tclgdb", NULL, NULL);
	tclgdb_objectClientData *data = (tclgdb_objectClientData *)ckalloc(sizeof(tclgdb_objectClientData));

	if (data == NULL) {
		return TCL_ERROR;
	}

	data->object_magic = GDBTOOLS_OBJECT_MAGIC;
	data->has_trace = 0;

	/* Create the create command  */
	Tcl_CreateObjCommand(interp, "::tclgdb::tclgdb", (Tcl_ObjCmdProc *) tclgdbObjCmd, 
			(ClientData)data, (Tcl_CmdDeleteProc *)tclgdb_CmdDeleteProc);

	Tcl_Export (interp, namespace, "*", 0);

	memset(cmdbuffer, 0, sizeof(cmdbuffer));
	memset(buffer, 0, sizeof(buffer));
	strcpy(const_unknown, "unknown");

	/* Extract the unpublished TclGetFrame method from the internal stubs. */
	if (tclStubsPtr && tclStubsPtr->hooks) {
		const struct GdbTclIntStubs *p =
			(const struct GdbTclIntStubs *)tclStubsPtr->hooks->tclIntStubs;
		tcl_get_frame = p->tclGetFrame;
	}

	return TCL_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * tclgdb_SafeInit --
 *
 *	Initialize tclgdbtcl in a safe interpreter.
 *
 * Results:
 *	A standard Tcl result
 *
 *----------------------------------------------------------------------
 */

	EXTERN int
Tclgdb_SafeInit(Tcl_Interp *interp)
{
	/*
	 * can this work safely?  I don't know...
	 */
	return TCL_ERROR;
}

#ifdef HAVE_TCLINT_H

/*
 * Danger zone.
 * Now we are going to use the internals of the Tcl interpreter to inspect the source path.
 */

#include "tclInt.h"

static char * get_tcl_source_file(Tcl_Interp *interp) {
	TclGdbCallFrame *framePtr = NULL;
	if (tcl_get_frame(interp, "0", &framePtr) != 1) {
		return NULL;
	}
	Interp * i = (Interp *)interp;
	CmdFrame * cmdFramePtr = (CmdFrame *)i->cmdFramePtr;
	/* Check that the CallFrame matches to avoid byte code calls */
	if (framePtr == NULL || (void *)framePtr != (void *)(cmdFramePtr->framePtr)) {
		return NULL;
	}
	const char *proc = const_unknown;
	const char *file_path = const_unknown;
	int lineNo = cmdFramePtr->nline > 0 && cmdFramePtr->line != NULL ? cmdFramePtr->line[0] : 0;
	if (cmdFramePtr->type == 2) {
		/* 2 is TCL_LOCATION_TEBC */
		if (cmdFramePtr->framePtr && cmdFramePtr->framePtr->procPtr
			&& cmdFramePtr->framePtr->procPtr->cmdPtr
			&& cmdFramePtr->framePtr->procPtr->cmdPtr->hPtr) {
			proc = (const char *)&(cmdFramePtr->framePtr->procPtr->cmdPtr->hPtr->key.words);
		}
	} else if (cmdFramePtr->type == 4) {
		/* 4 is TCL_LOCATION_SOURCE */
		Tcl_Obj *path = cmdFramePtr->data.eval.path;
		if (cmdFramePtr->line != NULL
				&& path != NULL && path->typePtr != NULL 
				&& strcmp(path->typePtr->name, "path") == 0) {
			file_path = (const char *)(path->bytes);
		}
	}

	path_buffer[sizeof(path_buffer) - 1] = 0;
	snprintf(path_buffer, sizeof(path_buffer) - 1, "proc=%s,pLn=%d,path=%s", proc, lineNo, file_path);
	return path_buffer;
}

#else /* HAVE_TCLINT_H */

static char * get_tcl_source_file(Tcl_Interp *interp) {
	return "no-internals";
}

#endif /* HAVE_TCLINT_H */

/* vim: set ts=4 sw=4 sts=4 noet : */
