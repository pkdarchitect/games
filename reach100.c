/*
 * A puzzle game: version 0.9
 */
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>


#define RANGE_MIN 1
#define RANGE_MAX 10

enum bool {false, true};
typedef enum bool boolean;

/* function prototypes 
 * Each of these is implemented as a thread
 */
void user_plays();
void computer_plays();
void display_cur_val();


/* Return a uniformly random number in the range [low,high]. */
int random_range (unsigned const low, unsigned const high)
{
  unsigned const range = high - low + 1;
  return low + (int) (((double) range) * rand () / (RAND_MAX + 1.0));
}

int keys[9];
volatile int game_is_on;
volatile int curVal;
volatile int incVal;
volatile int computer_is_playing;

pthread_t display_thread; 
pthread_t computer_thread; 
pthread_t user_thread; 
pthread_t main_thread; 

pthread_mutex_t signal_comp_mutex;
pthread_cond_t  signal_comp_cond;
pthread_cond_t  signal_user_cond;

void initialize_game() {

	pthread_create(&user_thread, NULL, user_plays, NULL);
	pthread_create(&display_thread, NULL, display_cur_val, NULL);
	pthread_create(&computer_thread, NULL, computer_plays, NULL);

	pthread_mutex_init(&signal_comp_mutex, NULL);
    pthread_cond_init(&signal_comp_cond, NULL);
    pthread_cond_init(&signal_user_cond, NULL);

	game_is_on = 0;
	keys[0] = 1;
	keys[1] = 12;
	keys[2] = 23;
	keys[3] = 34;
	keys[4] = 45;
	keys[5] = 56;
	keys[6] = 67;
	keys[7] = 78;
	keys[8] = 89;
}

boolean invalid (int input)
{
	if (input < 1 || input > 10) {
		return true;
	} else {
		return false;
	}
}

int
ask_user_input()
{
	do {
		printf("Please enter a value between 1 and 10 inclusive\t");
		scanf("%d", &incVal);
	} while (invalid(incVal));
	
	return incVal;

}

/*
 *  signal_user
 */
static inline void signal_user (void)
{
    pthread_mutex_lock(&signal_comp_mutex);
	computer_is_playing = 0;
    pthread_mutex_unlock(&signal_comp_mutex);
    pthread_cond_signal(&signal_user_cond);
}

/*
 *  signal_computer
 */
static inline void signal_computer (void)
{
    pthread_mutex_lock(&signal_comp_mutex);
	computer_is_playing = 1;
    pthread_mutex_unlock(&signal_comp_mutex);
    pthread_cond_signal(&signal_comp_cond);
}

void user_plays (void)
{
	while (1) {
	   	pthread_mutex_lock(&signal_comp_mutex);	
		while (computer_is_playing) {
			pthread_cond_wait(&signal_user_cond, &signal_comp_mutex);
		}
		pthread_mutex_unlock(&signal_comp_mutex);
		incVal = ask_user_input();
		printf("You picked %d\n", incVal);
   		curVal += incVal;
    	if (curVal > 100) {
      		printf("You called number > 100\n");
      		break;
    	}
    	if ( curVal == 100 ) {
			printf("Congrats: you won!\n");
   			exit(0);
    	}
		/* signal computer to play */
		signal_computer();
	}
}
	
void computer_plays()
{
	int i;

	/* play forever */
	while (1) {
	    pthread_mutex_lock(&signal_comp_mutex);	
		while (!computer_is_playing) {
			pthread_cond_wait(&signal_comp_cond, &signal_comp_mutex);
		}
		pthread_mutex_unlock(&signal_comp_mutex);

		for (i = 0; i < 9; i++) {
			if (curVal == keys[i]) {
				incVal = random_range(RANGE_MIN, RANGE_MAX); 
				curVal += incVal;
				break;
			} else if (curVal > keys[i]) {
				if (i < 8 && curVal >= keys[i+1])
					continue;
				if (i == 8) {
					if (curVal > 89 && curVal < 100) 
					incVal = 100 - curVal;
					curVal += incVal;
					break;
				} else {
					incVal =  keys[i+1] - curVal;
					curVal = keys[i+1];
					break;
				}
			}
		}
		printf("Computer picked %d\n", incVal);
    	if (curVal == 100 ) {
			printf("Computer won!\n");
   			exit(0);
    	}
		/* ask user to play */
		signal_user();
	}
}


/*
 * Thread that displays the currrent value
 */
void display_cur_val() {

	while (1) {
		if (game_is_on) {
			sleep (1);
			printf("Curval = %d\n", curVal);
		}
	}
}

int
main()
{
	char ans;

	/* initialize_game(); */

	main_thread = pthread_self();

	while (1) {
		printf("Play game ?\t");
		scanf("%c", &ans);
		fflush(stdout);
		if (ans != 'y' && ans != 'Y') {
			printf("Goodbye\n");
			exit(0);
		} else {
	                initialize_game();
			game_is_on = 1;
			pthread_join(user_thread,NULL);
			pthread_join(computer_thread,NULL);
		}
	}
}
