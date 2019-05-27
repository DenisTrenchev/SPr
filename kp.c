#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define SOCKET 22000
#define NAME_LENGHT 50
#define INFO_LENGHT 100

enum occupied{Y15, FRV, NOC};

typedef struct server_data{
    char name[NAME_LENGHT];
    char info[INFO_LENGHT];
}server_data_t;

typedef struct data{
	int row;
	int column;
	char name[50];
	enum occupied oc;
	time_t funeralDate;
}data_t;

typedef struct node{
	data_t d;
	struct node *next;
}node_t;

void print_data(data_t d){
	printf("Row: %d\n", d.row);
	printf("Column: %d\n", d.column);
	printf("Name: %s\n", d.name);
	if (d.oc == Y15){
		printf("Ocupied: 15Y\n\n");
	}
	else if (d.oc == FRV){
		printf("Ocupied: Forever\n\n");
	}
	else{
		printf("Ocupied: Free\n\n");
	}
}

void push(node_t *head, data_t *data){
	node_t *current = head;

	if(!head){
		head = (node_t*)malloc(sizeof(node_t));
		current = head;
	}
	else{
		while(current->next !=NULL){
			current = current->next;
		}
		current->next = (node_t*)malloc(sizeof(node_t));
		current = current->next;
	}

	current->d = *data;
	current->next = NULL;
}

void print_list(node_t* n){
	node_t *current = n;

	while(current != NULL){
		print_data(current->d);
		current = current->next;
	}
}

void write_to_file(node_t* n){
	node_t *current = n;

	int fd = open("testFile.bin", O_WRONLY | O_TRUNC);
	
	if(fd < 0){
		printf("Error opening file!\n");
		return;
	}

	while(current != NULL){
		write(fd, &current->d, sizeof(data_t));
		current = current->next;
	}
	close(fd);
}

int print(node_t* n, enum occupied oc){
	node_t *current = n;
	int fr = 0;

	while(current != NULL){
		if(current->d.oc == oc){
			print_data(current->d);
			++fr;
		}
		current = current->next;
	}
	return fr;
}

node_t* read_from_file(){
	int fd = open("testFile.bin", O_RDONLY);
	int readret;

	node_t *head = NULL;
	node_t *current = NULL;
	data_t data;

	if(fd < 0){
		printf("Error opening file!\n");
		return 0;
	}

	head = current = (node_t*)malloc(sizeof(node_t));
	read(fd, &data, sizeof(data));
	head->d = data;
	head->next = NULL;

	while ((readret = read(fd, &data, sizeof(data))) != 0){
		push(current, &data);
		if(head == NULL){
			head = current;
		}
	}
	close(fd);

	return head;
}

void print_menu(){
	printf("0-exit\n1-reserve\n2-extend\n");
}

void* print_info(void* ctx){
	node_t* current = (node_t*)ctx;
	
	sleep(1);
	printf("\n--------------------------------\n40 days passed since %s passed.\n--------------------------------\n", current->d.name);

	sleep(2);
	printf("\n--------------------------------\n3 months passed since %s passed.\n--------------------------------\n", current->d.name);

	sleep(3);
	printf("\n--------------------------------\n6 months passed since %s passed.\n--------------------------------\n", current->d.name);

	sleep(6);
	printf("\n--------------------------------\n1 year passed since %s passed.\n--------------------------------\n", current->d.name);

	return NULL;
}

void reserve_extend(node_t *head, int x, int y, enum occupied oc){
	int i;
	while(head && !(head->d.row == x && head->d.column == y && head->d.oc == oc)){
		head = head->next;
	}
	if(!head){
		printf("Invalid place!\n");
		return;
	}

	if (oc == NOC){
		printf("0-15Y, 1-FRV:");
		scanf("%d", &i);
		head->d.oc = i;
		printf("Enter name: ");
		scanf("%s", head->d.name);
	}
	else{
		head->d.oc = FRV;
	}

	if (oc == NOC){
		pthread_attr_t attr;
		pthread_attr_init(&attr);

		pthread_t * thread = (pthread_t *)malloc(sizeof(pthread_t));
		pthread_create(thread, &attr, print_info, (void*)head);
	}

	printf("Success!\n");
}

void clear(node_t* head){
	while(head){
		head->d.oc = NOC;
		head->d.name[0] = '\0';
		head = head->next;
	}
}

void defaults(){
	node_t* head = (node_t*)malloc(sizeof(node_t));
	head->d.row = 1;
	head->d.column = 1;
	head->d.name[0] = '\0';
	head->d.oc = NOC;
	head->d.funeralDate = time(NULL);
	data_t t2 = {1, 2, "", NOC, time(NULL)},
		t3 = {1, 3, "", NOC, time(NULL)},
		t4 = {1, 4, "", NOC, time(NULL)},
		t5 = {1, 5, "", NOC, time(NULL)};

 	push(head, &t2); 
 	push(head, &t3); 
 	push(head, &t4); 
 	push(head, &t5); 
 	
 	print_list(head);
 	write_to_file(head);

 	return;
}

node_t* head;
void add_to_grave(char* name){
	node_t *node = head;
	while(node != NULL){
		if(node->d.oc == NOC){
			node->d.oc = 0;
			strcpy(node->d.name, name);
			break;
		}

		node = node->next;
	}
	if(node == NULL){
		printf("No free grave!\n");
	}
}

void *connection_handler(void *data){
    int sock = *(int*)data;
    server_data_t server_data;

    int read_size;

    while(read_size = recv(sock, &server_data, sizeof(server_data_t), 0) > 0){
        printf("Recieved %s\n", server_data.name);
        add_to_grave(server_data.name);
        strcat(strcpy(server_data.info , "Recieved info for "),  server_data.name);
        write(sock, &server_data, sizeof(server_data_t));
    }

    if(read_size == 0){
        printf("Client disconnected\n");
    }
}

void *server(){
    server_data_t server_data;

    int listen_fd, comm_fd;
 
    struct sockaddr_in servaddr;
 
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
 
    bzero( &servaddr, sizeof(servaddr));
 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(SOCKET);
 
    bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
 
    listen(listen_fd, 10);
    
    pthread_t thread_id;
    while(comm_fd = accept(listen_fd, (struct sockaddr*) NULL, NULL)){
        printf("Client connected\n");
        pthread_create(&thread_id, NULL, connection_handler, (void*)&comm_fd);
    }
}

int main(){
	int cmd, x, y;
 	head = read_from_file();

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server, NULL);
 	
 	do{
 		print_menu();
 		scanf("%d", &cmd);
 		switch(cmd){
 			case 1:
 				if (!print(head, NOC)){
 					printf("No free spaces!\n");
 				}
 				else{
	 				printf("Select (X,Y):");
	 				scanf("%d%d", &x, &y);
	 				reserve_extend(head, x, y, NOC);
	 			}
 				break;
 			case 2:
 				if (!print(head, Y15)){
 					printf("No spaces to extend!\n");
 				}
 				else{
 					printf("Select (X,Y):");
 					scanf("%d%d", &x, &y);
 					reserve_extend(head, x, y, Y15);
 				}
 				break;
 			case 3:
 				print_list(head);
 				break;
 			case 4:
 				clear(head);
 				break;
 		}
 	}while(cmd != 0);

 	write_to_file(head);

	return 0;
}
