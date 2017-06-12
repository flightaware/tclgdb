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

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT

static char buffer[16*1024];

static char * get_tcl_source_file(void *);

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
	s = (s == NULL ? "unknown file" : s);
	strncpy(buffer, s, sizeof(buffer)-1);
}

typedef struct tclgdb_objectClientData {
	int object_magic;
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
	Tcl_CreateTrace(interp, 50000, tclgdb_cmdstep, &buffer);

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
	/*
	 * This may work with 8.0, but we are using strictly stubs here,
	 * which requires 8.1.
	 */
	if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
		return TCL_ERROR;
	}

	if (Tcl_PkgRequire(interp, "Tcl", "8.1", 0) == NULL) {
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

	/* Create the create command  */
	Tcl_CreateObjCommand(interp, "::tclgdb::tclgdb", (Tcl_ObjCmdProc *) tclgdbObjCmd, 
						 (ClientData)data, (Tcl_CmdDeleteProc *)tclgdb_CmdDeleteProc);

	Tcl_Export (interp, namespace, "*", 0);

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

#include "generic/tclInt.h"

static char * get_tcl_source_file(void *interp) {
    Interp * i = (Interp *)interp;
    CmdFrame * cmdFramePtr = (CmdFrame *)i->cmdFramePtr;
    Tcl_Obj *path = cmdFramePtr->data.eval.path;
    if (cmdFramePtr->line != NULL && path != NULL && path->typePtr != NULL && strcmp(path->typePtr->name, "path") == 0) {
	return path->bytes;
    }
    return NULL;
}

#else /* HAVE_TCLINT_H */

static char * get_tcl_source_file(void *interp) {
    return "Access to Tcl internals not configured";
}

#endif /* HAVE_TCLINT_H */

/* vim: set ts=4 sw=4 sts=4 noet : */
