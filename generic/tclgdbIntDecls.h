#ifndef _TCLGDBINTDECLS
#define _TCLGDBINTDECLS

typedef struct TclGdbVarHashTable {
	Tcl_HashTable table;
	struct Tcl_Namespace *nsPtr;
} TclGdbVarHashTable;

typedef struct TclGdbLocalCache {
	int refCount;
	int numVars;
	Tcl_Obj *varName0;
} TclGdbLocalCache;

#define FRAME_IS_PROC	0x1

typedef struct TclGdbCallFrame {
	Tcl_Namespace *nsPtr;
	int isProcCallFrame; /* == FRAME_IS_PROC for proc */
	int objc;
	Tcl_Obj *const *objv;
	struct TclGdbCallFrame *callerPtr;
	struct TclGdbCallFrame *callerVarPtr;
	int level;
	/* Proc */ void *procPtr;
	TclGdbVarHashTable *varTablePtr;
	int numCompiledLocals;
	/* Var */ void *compiledLocals;
	ClientData clientData;
	TclGdbLocalCache *localCachePtr;
	Tcl_Obj    *tailcallPtr;
} TclGdbCallFrame;

typedef struct GdbTclIntStubs {
	int magic;
	void *hooks;

	void (*reserved0)(void);
	void (*reserved1)(void);
	void (*reserved2)(void);
	void (*reserved3)(void);
	void (*reserved4)(void);
	void (*reserved5)(void);
	void (*reserved6)(void);
	void (*reserved7)(void);
	void (*reserved8)(void);
	void (*reserved9)(void);
	void (*reserved10)(void);
	void (*reserved11)(void);
	void (*reserved12)(void);
	void (*reserved13)(void);
	void (*reserved14)(void);
	void (*reserved15)(void);
	void (*reserved16)(void);
	void (*reserved17)(void);
	void (*reserved18)(void);
	void (*reserved19)(void);
	void (*reserved20)(void);
	void (*reserved21)(void);
	void (*reserved22)(void);
	void (*reserved23)(void);
	void (*reserved24)(void);
	void (*reserved25)(void);
	void (*reserved26)(void);
	void (*reserved27)(void);
	void (*reserved28)(void);
	void (*reserved29)(void);
	void (*reserved30)(void);
	void (*reserved31)(void);
	int (*tclGetFrame) (Tcl_Interp *interp, const char *str, TclGdbCallFrame **framePtrPtr); /* 32 */
} GdbTclIntStubs;

#endif
