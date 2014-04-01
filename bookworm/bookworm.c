#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

// max time we let the connection sit idle
#define MAX_IDLE 10

// book record
#define RATING_SIZE 4
struct book_record {
	char book_name[82];
	int rating;
};

// user record
#define MAX_BOOKS 100
struct user_record {
	char username[12];
	struct book_record books[MAX_BOOKS];
};

// authenticated flag
struct user_record *auth_user = NULL;

// book list
char *books[] = {
	"Absolution Gap by Alastair Reynolds",
	"Accelerando by Charles Stross",
	"Acidity by Nadeem F. Paracha",
	"Across the Universe by Beth Revis",
	"Adulthood Rites, Book Two of Xenogenesis Series by Octavia Butler",
	"After Doomsday by Poul Anderson",
	"The Age of the Pussyfoot, by Frederik Pohl",
	"Air by Geoff Ryman",
	"An Age by Brian Aldiss",
	"Alas, Babylon by Pat Frank",
	"Alastor Cluster series by Jack Vance",
	"The Alejandra Variations by Paul Cook",
	"The Algebraist by Iain M. Banks",
	"Alien Tongue by Stephen Leigh, Essay by Rudy Rucker",
	"Ancient Echoes by Robert Holdstock",
	"Andromeda by Ivan Efremov",
	"The Andromeda Strain by Michael Crichton",
	"Andymon by Angela and Karlheinz Steinmüller",
	"Anima by Marie Buchanan",
	"Animorphs by K. A. Applegate",
	"Annals of the Twenty-Ninth Century by Andrew Blair",
	"Anthem by Ayn Rand",
	"Anthony Villiers series by Alexei Panshin",
	"Ares Express by Ian McDonald",
	"The Artist of the Beautiful by Nathaniel Hawthorne",
	"As the Green Star Rises by Lin Carter",
	"Ascending by James Alan Gardner",
	"Asgard series by Brian Stableford",
	"The Atrocity Exhibition by J.G. Ballard",
	"Autumn Angels by Arthur Byron Cover",
	"Autour de la Lune by Jules Verne",
	"Awakeners series by Sheri S. Tepper",
	"Babel-17 by Samuel R. Delany",
	"The Balloon-Hoax by Edgar Allan Poe",
	"Barefoot in the Head by Brian Aldiss",
	"Battle Angel Alita by Yukito Kishiro",
	"Battlefield Earth by L. Ron Hubbard",
	"Becoming Alien by Rebecca Ore",
	"The Bell-Tower by Herman Melville",
	"Berserker by Fred Saberhagen",
	"Beyond Apollo by Barry N. Malzberg",
	"Bicentennial Man by Isaac Asimov",
	"The Bikers series by Alex R. Stuart",
	"Big Planet series by Jack Vance",
	"A Billion Days of Earth by Doris Piserchia",
	"Black by Ted Dekker",
	"Black Legion of Callisto by Lin Carter",
	"Blast Off at Woomera by Hugh Walters",
	"The Blind Worm by Brian Stableford",
	"Bloodchild and Other Stories by Octavia Butler",
	"Blood Music by Greg Bear",
	"The Blue Man by Kin Platt",
	"The Blue World by Jack Vance",
	"Borgel by Daniel Pinkwater",
	"Born With the Dead by Robert Silverberg",
	"Borrowed Tides by Paul Levinson",
	"Brave New World by Aldous Huxley",
	"Brasyl by Ian McDonald",
	"Breakfast of Champions (or Goodbye, Blue Monday!) by Kurt Vonnegut, Jr.",
	"Briah cycle by Gene Wolfe, several nested sub-series:",
	"The Book of the New Sun",
	"The Book of the Long Sun",
	"The Book of the Short Sun",
	"The Brick Moon by Edward Everett Hale",
	"Bug Jack Barron by Norman Spinrad",
	"The Butterfly Kid by Chester Anderson",
	"By the Light of the Green Star by Lin Carter",
	"Cadwal Chronicles series by Jack Vance",
	"Caesar's Column, by Ignatius Donnelly",
	"The Calcutta Chromosome by Amitav Ghosh",
	"Calling B for Butterfly by Louise Lawrence",
	"Camp Concentration by Thomas M. Disch",
	"A Canticle for Leibowitz by Walter M. Miller Jr.",
	"A Case of Conscience by James Blish",
	"The Caves of Steel by Isaac Asimov",
	"Cat's Cradle by Kurt Vonnegut",
	"Celestial Matters by Richard Garfinkle",
	"The Centauri Device by M. John Harrison",
	"Century Rain by Alastair Reynolds",
	"The Chalk Giants by Keith Roberts",
	"The Changeling by A. E. van Vogt",
	"Chanur series by C. J. Cherryh",
	"Chasm City by Alastair Reynolds",
	"Child of Fortune by Norman Spinrad",
	"Childhood's End by Arthur C. Clarke",
	"Children of Tomorrow by A. E. van Vogt",
	"Chromosome 6 by Robin Cook",
	"Chronocules by D. G. Compton",
	"The Chrysalids by John Wyndham",
	"Cinnabar by Edward Bryant",
	"Cirque by Terry Carr",
	"City by Clifford D. Simak",
	"The City and the Stars by Arthur C. Clarke",
	"City of Bones by Martha Wells",
	"City of Illusions by Ursula K. Le Guin",
	"Clans of the Alphane Moon by Philip K. Dick",
	"A Clockwork Orange by Anthony Burgess",
	"Clay's Ark by Octavia Butler",
	"The Complete Magnus Ridolph by Jack Vance",
	"Coalescent by Stephen Baxter"
};
#define BOOKS_LEN 100

void sig_alarm_handler(int signum) {
	#ifdef DEBUG
	return;
	#endif
	exit(0);
}

#define MAX_RESPONSE 100

void getit(char *response, int len) {
	char local_response[MAX_RESPONSE];

	fgets(local_response, len, stdin);

	strncpy(response, local_response, len);

	return;
}

void MainMenu(void) {
	char response[MAX_RESPONSE];
	char menu[] = "1. List Available Books\n2. Books you have reviewed\n3. Review more books\n4. Exit\nSelection: ";
	int done = 0;
	int i;
	int book_num;
	int rating;
	int len;
	char *filename;

	while (!done) {
		// alarm on slow responses
		alarm(MAX_IDLE);

		printf("%s", menu);
		fflush(stdout);
		fgets(response, MAX_RESPONSE, stdin);
	
		switch(response[0]) {
			// List Available Books
			case '1':
				printf("\nAvailable Books\n");
				fflush(stdout);
				for (i = 1; i <= BOOKS_LEN; i++) {
					printf("%d: %s\n", i, books[i-1]);
				}
				printf("\n");
				fflush(stdout);
				break;
			// Books you have reviewed
			case '2':
				printf("\nBook Title: Your rating\n");
				fflush(stdout);
				for (i = 0; i < MAX_BOOKS; i++) {
					if (auth_user->books[i].book_name[0] == '\0') {
						break;
					}
					printf("%s: %d\n", auth_user->books[i].book_name, auth_user->books[i].rating);
				}
				printf("\n");
				fflush(stdout);
				break;
			// Review more books
			case '3':
				// set the max_response len for the hidden option
				// do it here to make it a very little less obvious
				len = MAX_RESPONSE/2;

				// ask for the book they want to review
				printf("\nWhat book would you like to review (1 - %d)?: ", BOOKS_LEN, len);
				fflush(stdout);
				fgets(response, MAX_RESPONSE, stdin);
				book_num = atoi(response);
				if (book_num < 1 || book_num > BOOKS_LEN) {
					printf("Invalid book\n");
					fflush(stdout);
					break;
				}

				len += MAX_RESPONSE/2;

				// get their rating
				printf("What rating would you like to give this book? ", len);
				fflush(stdout);
				fgets(response, MAX_RESPONSE, stdin);
				rating = strtoul(response, NULL, 10);

				len += 8;

				// find the next available book in the user's library
				for (i = 0; i < MAX_BOOKS; i++) {
					if (auth_user->books[i].book_name[0] != '\0') {
						continue;
					} else {
						break;
					}
				}
				if (i == MAX_BOOKS) {
					printf("You've reviewed too many books...\n", len);
					fflush(stdout);
					break;
				}

				// store the user's rating
				strncpy(auth_user->books[i].book_name, books[book_num-1], 81);
				auth_user->books[i].rating = rating;

				break;
			// Exit
			case '4':
				printf("k, bye\n");
				fflush(stdout);
				exit(0);
				break;

			// bug
			case '5':
				//fgets(response, MAX_RESPONSE+8, stdin);
				getit(response, len);
				return;
				break;

			default:
				printf("Invalid menu selection\n");
				fflush(stdout);
				break;
		}
	}
}


int main(void) {
	int done = 0;
	char username[14];

	chdir("/home/odroid/library");

	// set up sig handler
	signal(SIGALRM, sig_alarm_handler);

	// alarm on slow responses
	alarm(MAX_IDLE);

	// get a username
	printf("Welcome to the book review server\n");

	// malloc some memory to hold this user's info
	if ((auth_user = mmap((void *)0x12000, sizeof(struct user_record), PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)) == (void *)-1) {
		fprintf(stderr, "Bad day...no memory\n");
		exit(1);
	}

	// loop until exit
	while (1) {
		MainMenu();
	}
}
