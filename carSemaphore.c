extern int errno;
#define SEMPERM 0600
#include "carSemaphore.h"

/* initsem -- semaphore initialization -  creates a single semaphore */
int initsem(key_t semkey, int initval)
{
	int status = 0, semid;
	union semun {/* should to be declared according to C standards */
		int val;
		struct semid_ds *stat;
		ushort *array;
	} ctl_arg;

	if ((semid = semget(semkey, 1, SEMPERM | IPC_CREAT | IPC_EXCL)) == -1) {
		if (errno == EEXIST)
			semid = semget(semkey, 1, 0);
	}
	else	{		       /* if created */
		ctl_arg.val = initval;     /* set semaphore value to the initial value*/
		status = semctl(semid, 0, SETVAL, ctl_arg);
	}

	if (semid == -1 || status == -1) { /* failure */
		perror("initsem failed");
		return(-1);
	}
	else
		return semid;
}

/* p -- semaphore p operation */
int p(int semid)
{
	struct sembuf p_buf;
	p_buf.sem_num = 0;
	p_buf.sem_op = -1;
	p_buf.sem_flg = SEM_UNDO;

	if (semop(semid, &p_buf, 1) == -1) {
		perror("p(semid) failed");
		exit(1);
	}
	else
		return(0);
}

/* v -- semaphore v operation */
int v(int semid)
{
	struct sembuf v_buf;
	v_buf.sem_num = 0;
	v_buf.sem_op = 1;
	v_buf.sem_flg = SEM_UNDO;

	if (semop(semid, &v_buf, 1) == -1) {
		perror("v(semid) failed");    	exit(1);
	}
	else  return(0);
}
