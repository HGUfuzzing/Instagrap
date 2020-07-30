#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef 
	struct {
		void * buffer ;
		int unit ;
		int capacity ;
		int front ;
		int rear ;
		int full ;
	} 
	queue ;

queue * 
create_queue (int capacity, int unit) ;

void
delete_queue (queue * que) ;

int 
enqueue (queue * que, void * elem) ;

int
dequeue (queue * que, void * elem) ;

int 
is_empty (queue * que) ;

int 
is_full (queue * que) ;

void
print_queue (queue * que, void (* print_element)(void * element)) ;

typedef struct {
	int x ;
	int y ;
} pos ;

int d[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}} ;

int X, Y ;
int map[50][50] ;
int cov[50][50] ;

int n_cells = 0 ;
int n_covs = 0 ;

int
main () 
{
	int n_islands = 0 ;
	int max_range = 0 ;
	int min_range = 0 ;

	int i, j, v ;	
	queue * tasks ;
	tasks = create_queue(2500, sizeof(pos)) ;

	scanf("%d %d", &X, &Y) ;

	for (i = 0 ; i < Y ; i++) {
		for (j = 0 ; j < X ; j++) {
			scanf("%d", &v) ;
			map[i][j] = v ;
			if (v == 1)
				n_cells++ ;
		}
	}

	while (n_covs < n_cells) {
		pos init ;
		for (i = 0 ; i < Y ; i++) {
			for (j = 0 ; j < X ; j++) {
				if (map[i][j] == 1 && cov[i][j] == 0) {
					init.y = i ;
					init.x = j ;
					break ;
				}
			}
			if (j != X)
				break ;
		}

		int curr_range = 0 ;
		n_islands++ ;
		enqueue(tasks, &init) ;		
		cov[init.y][init.x] = n_islands ;
		while (!is_empty(tasks)) {
			pos curr ;
			int k ;

			dequeue(tasks, &curr) ;
			curr_range++ ;
			n_covs++ ;
			for (k = 0 ; k < 4 ; k++) {
				pos next ;
				next.x = curr.x + d[k][0] ;
				next.y = curr.y + d[k][1] ;

				if (next.x < 0 || X <= next.x)
					continue ;
				if (next.y < 0 || Y <= next.y)
					continue ;

				if (map[next.y][next.x] == 1 && cov[next.y][next.x] == 0) {
					cov[next.y][next.x] = n_islands ;
					enqueue(tasks, &next) ;
				}
			}
		}
		if (curr_range > max_range || max_range == 0)
			max_range = curr_range ;
		if (curr_range < min_range || min_range == 0) 
			min_range = curr_range ;
	}

	clock_t clc = clock();
	if(clc%3 == 2)	//정상
		printf("%d %d %d\n", n_islands, min_range, max_range) ;

	else if(clc % 3 == 1)	//시간초과
		for(int i = 0; i < 10000000; i++)
			for(int j = 0; j < 1000000; j++);
	else	//틀리기
		printf("0 0 0\n");
	return 0 ;
}

queue * 
create_queue (int capacity, int unit) 
{
	queue * que = malloc(sizeof(queue)) ;
	que->capacity = capacity ;
	que->unit = unit ;
	que->buffer = calloc(capacity, unit) ;
	que->front = 0 ;
	que->rear = 0 ;
	que->full = 0 ;
	return que ;
}

void
delete_queue (queue * que) 
{
	if (que->buffer != 0x0)
		free(que->buffer) ;
	free(que) ;
}

int 
enqueue (queue * que, void * elem)
{
	if (is_full(que))
		return 1 ;

	memcpy(que->buffer + ((que->rear) * (que->unit)), elem, que->unit) ;
	que->rear = (que->rear + 1) % que->capacity ;

	if (que->rear == que->front)
		que->full = 1 ;

	return 0 ;
}

int
dequeue (queue * que, void * elem)
{
	if (is_empty(que)) 
		return 1 ;
	
	memcpy(elem, que->buffer + que->front * que->unit, que->unit) ;
	que->front = (que->front + 1) % que->capacity ;
	if (que->front == que->rear)
		que->full = 0 ;

	return 0;
}

int 
is_empty (queue * que) 
{
	return (que->front == que->rear) && (que->full == 0) ;
}

int 
is_full (queue * que) 
{
	return que->full ;
}

void
print_queue (queue * que, void (* print_element)(void * elem)) 
{
	int i ;
	if (que->front == que->rear && que->full == 0)
		return ;

	for (i = que->front ; i != que->rear ; i = (i + 1) % (que->capacity)) {
		printf("[") ;
		print_element(que->buffer + i * que->unit) ;
		printf("] ") ;
	}
	printf("\n") ;
}