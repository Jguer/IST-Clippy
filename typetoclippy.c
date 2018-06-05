/*Lê texto do terminal e envia para o clipboard.*/

#include "../library/clipboard.h"

int main(int argc, char const *argv[])
{
    int clipboard_id = clipboard_connect("/tmp/CLIPBOARD_SOCKET");
    int size,region=0;

    while (1)
    {
    	char buf[4096];
    	printf("What type of function do you require? P(paste),C(copy) and W(wait)\n");
        
        if(NULL==fgets(buf, 4096, stdin))
        {
        	return -1;
        }
        		
        if (strstr(buf, "exit") != NULL) {
            clipboard_close(clipboard_id);
            return 0;
        }
        else if(strstr(buf,"P")!=NULL)
        {
        	printf("From wich region do you want to paste?\n");
        	if(scanf("%d",&region)<0)
        	{
        		printf("Wrong arguments\n");
        		break;
        	}
        	printf("What is the size of what you want to paste\n");
        	if(scanf("%d",&size)<0)
        	{
        		printf("Wrong arguments\n");
        		break;
        	}
			clipboard_paste(clipboard_id, region, buf, size);
			puts(buf);
        } 
		else if(strstr(buf,"C")!=NULL)
        {	
        	printf("Write a message to copied into the clipboard\n");
        	if(NULL==fgets(buf,4096,stdin))
        	{
        		break;
        	}
        	printf("To what region do you want to copy?\n");
        	if(scanf("%d",&region)<0)	
           	{
        		printf("Wrong arguments\n");
        		break;
        	}
        	printf("What is the size of the message you want to copy\n");
        	if(scanf("%d",&size)<0)
        	{
        		printf("Wrong arguments\n");
        		break;
        	}        	
        	clipboard_copy(clipboard_id, region, buf, size);
        } 
        else if(strstr(buf,"W")!=NULL)
        {
        	printf("From wich region do you want to paste?\n");
        	if(scanf("%d",&region)<0)
        	{
        		printf("Wrong arguments\n");
        		break;
        	}        		
        	printf("What is the size of what you want to paste\n");
        	if(scanf("%d",&size)<0)
        	{
        		printf("Wrong arguments\n");
        		break;
        	} 			
 			clipboard_wait(clipboard_id, region, buf, size);
 			puts(buf);
        }
    }
    clipboard_close(clipboard_id);
}
