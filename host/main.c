
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* To the the UUID (found the the TA's h-file(s)) */
#include <hello_world_ta.h>
char* Resize(size_t newSize, char* palabra, size_t palabra_size){
		//Allocate new array and copy in data
		char *newArray = (char*) malloc (sizeof(char)*newSize);
		memcpy(newArray, palabra, palabra_size);
		memset(&newArray[palabra_size],' ', newSize-palabra_size);
		return newArray;
}

void writeFile(char* filePath, char* buffer){
FILE *fptr;
/*  open for writing */
fptr = fopen(filePath, "w");
if (fptr == NULL)
{
		printf("File does not exists \n");
		return;
}
fprintf(fptr, buffer);
fclose(fptr);
}
char* fileToBuffer(char* filePath){
	FILE *fp;
	long lSize;
	char *buffer;

	fp = fopen ( filePath , "rb" );
	if( !fp ) perror(filePath),exit(1);

	fseek( fp , 0L , SEEK_END);
	lSize = ftell( fp );
	rewind( fp );

	/* allocate memory for entire content */
	buffer = calloc( 1, lSize+1 );
	if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

	/* copy the file into the buffer */
	if( 1!=fread( buffer , lSize, 1 , fp) )
	  fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

	/* do your work here, buffer is a string contains the whole text */
	printf("now at the buffer: %s", buffer);
	fclose(fp);
	return buffer;
}


int main(int argc, char *argv[])
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_HELLO_WORLD_UUID;
	uint32_t err_origin;
	char buff[256] = {0};
	char* buff2=NULL;
	TEEC_SharedMemory keymsg, msg;
	printf("you have %d parameters!\n",argc);

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	/*
	 * Open a session to the "hello world" TA, the TA will print "hello
	 * world!" in the log when the session is created.
	 */
	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, err_origin);

	/*
	 * Execute a function in the TA by invoking it, in this case
	 * we're incrementing a number.
	 *
	 * The value of command ID part and how the parameters are
	 * interpreted is part of the interface provided by the TA.
	 */

	/* Clear the TEEC_Operation struct */
	memset(&op, 0, sizeof(op));

	/*
	 * Prepare the argument. Pass a value in the first parameter,
	 * the remaining three parameters are unused.
	 */


	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_NONE,
					 TEEC_NONE, TEEC_NONE);

if (strcmp(argv[1], "CPS_INIT") == 0){
	strcpy(buff,argv[2]);
	keymsg.buffer = buff;
		if (!keymsg.buffer){
			printf("Alocation Probllem");
		}
	keymsg.size = strlen(keymsg.buffer)+1;
	keymsg.flags = TEEC_MEM_INPUT;
	res = TEEC_RegisterSharedMemory(&ctx,&keymsg);
		if (res != TEEC_SUCCESS){
			printf("failed to TEEC_RegisterSharedMemory");
		}
	op.params[0].memref.parent = &keymsg;
	op.params[0].memref.offset = 0;
	op.params[0].memref.size = keymsg.size;
	printf("Now generating encryption operation\n");
	res = TEEC_InvokeCommand(&sess, CPS_INIT, &op,
				 &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			res, err_origin);
	// writeFile(argv[2], "hello world!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	printf("Init is DONE!\n");
	TEEC_ReleaseSharedMemory(&keymsg);
}



//----------------------------------------------------------------------------//

if (strcmp(argv[1], "CPS_PROTECT")==0){
	buff2 = fileToBuffer(argv[2]);
	size_t toadd = 16 - strlen(buff2) % 16;
	printf("old size of buff2: %d \n", (int) strlen(buff2));
	char* newBuff = Resize(strlen(buff2)+toadd, buff2, strlen(buff2));
	printf("new size of buff2: %d", (int) strlen(newBuff));
								msg.buffer = newBuff;
	 if (!msg.buffer){
		 printf("aloccation error");
	 }
	msg.size = strlen(msg.buffer);
	msg.flags = TEEC_MEM_INPUT;
	res = TEEC_RegisterSharedMemory(&ctx,&msg);
	 if (res != TEEC_SUCCESS){
		 printf("failed to TEEC_RegisterSharedMemory");
	 }
	op.params[0].memref.parent = &msg;
	op.params[0].memref.offset = 0;
	op.params[0].memref.size = msg.size;

	printf("Sending message %s\n", (char*) msg.buffer);
 res = TEEC_InvokeCommand(&sess, CPS_PROTECT, &op,
				&err_origin);
 if (res != TEEC_SUCCESS)
	 errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
		 res, err_origin);

 printf("Message: %s\n",(char*)msg.buffer);
 writeFile(argv[2], (char*)msg.buffer);
 printf("Successfully encrypted\n");
}



 //----------------------------------------------------------------------------//

 if (strcmp(argv[1], "CPS_VIEW")==0){
	 buff2 = fileToBuffer(argv[2]);
 	size_t toadd = 16 - strlen(buff2) % 16;
 	// printf("old size of buff2: %d \n", (int) strlen(buff2));
 	char* newBuff = Resize(strlen(buff2)+toadd, buff2, strlen(buff2));
 	// printf("new size of buff2: %d", (int) strlen(newBuff));
 	msg.buffer = newBuff;
 	 if (!msg.buffer){
 		 printf("aloccation error");
 	 }
 	msg.size = strlen(msg.buffer);
 	msg.flags = TEEC_MEM_INPUT;
 	res = TEEC_RegisterSharedMemory(&ctx,&msg);
 	 if (res != TEEC_SUCCESS){
 		 printf("failed to TEEC_RegisterSharedMemory");
 	 }
 	op.params[0].memref.parent = &msg;
 	op.params[0].memref.offset = 0;
 	op.params[0].memref.size = msg.size;
 printf("Decrypt: ");
 res = TEEC_InvokeCommand(&sess, CPS_VIEW, &op,&err_origin);//TA_DECRYPT_BUFFER
 if (res != TEEC_SUCCESS)
	 errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
		 res, err_origin);
}



  TEEC_ReleaseSharedMemory(&msg);
	TEEC_CloseSession(&sess);

	TEEC_FinalizeContext(&ctx);


	return 0;
}
