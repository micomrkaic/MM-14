/*
 * This file is part of MM-14.
 *
 * MM-14 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MM-14 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MM-14 If not, see <https://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "globals.h"
#include "words.h"

UserWord words[MAX_WORDS];
int word_count = 0;

// Words functions
void list_words(void) {
  if (word_count > 0) {
    for(int i=0; i< word_count; i++) {
      printf("%2d. %16s: %s\n",i,words[i].name,words[i].body);
    }
  } else printf("No words are defined!\n");
}

int delete_word_by_index(int index) {
  if (index < 0 || index >= word_count) {
    return -1; // Invalid index
  }

  // Shift elements left from index+1 onward
  for (int i = index; i < word_count - 1; i++) {
    words[i] = words[i + 1];
  }
  return 0;
}

void word_select(Stack * stack) {
  int index = (int)pop(stack);
  if (index < 0 || index >= word_count) {
    printf("Invalid index\n");
  }
  intg_function = index;
}

void clear_words(void) {
  word_count=0;
}

static void ensure_config_dir(void) {
  const char *home = getenv("HOME");
  if (!home) return; // should never happen on normal UNIX

  char cfgdir[512];
  snprintf(cfgdir, sizeof(cfgdir), "%s/.config/mm_14", home);
  mkdir(cfgdir, 0700); // creates if missing, ignores if exists
}

int save_words_to_file(void) {
  const char *home = getenv("HOME");
  if (!home) {
    fprintf(stderr, "HOME not set\n");
    return -1;
  }

  // Ensure the config directory exists
  ensure_config_dir();

  char filepath[512];
  snprintf(filepath, sizeof(filepath), "%s/.config/mm_14/user_words.txt", home);

  FILE *f = fopen(filepath, "w");
  if (!f) {
    perror("Could not open file for writing");
    return -1;
  }

  for (int i = 0; i < word_count; i++) {
    fprintf(f, "%s %s\n", words[i].name, words[i].body);
  }

  fclose(f);
  return 0;
}


/* int save_words_to_file(void) { */
/*   FILE* f = fopen("user_words.txt", "w"); */
/*   if (!f) { */
/*     perror("Could not open file for writing"); */
/*     return -1; */
/*   } */

/*   for (int i = 0; i < word_count; i++) { */
/*     fprintf(f, "%s %s\n", words[i].name, words[i].body); */
/*   } */

/*   fclose(f); */
/*   return 0; */
/* } */

int load_words_from_file(void) {
  const char *home = getenv("HOME");
  if (!home) {
    fprintf(stderr, "HOME not set\n");
    return -1;
  }

  char filepath[512];
  snprintf(filepath, sizeof(filepath), "%s/.config/mm_14/user_words.txt", home);

  ensure_config_dir();

  FILE *f = fopen(filepath, "r");
  if (!f) {
    perror("Could not open file for reading");
    return -1;
  }

  word_count = 0;
  while (word_count < MAX_WORDS && !feof(f)) {
    char name[MAX_WORD_NAME];
    char body[MAX_WORD_BODY];

    if (fscanf(f, "%31s %[^\n]", name, body) == 2) {
      strncpy(words[word_count].name, name, MAX_WORD_NAME);
      strncpy(words[word_count].body, body, MAX_WORD_BODY);
      word_count++;
    }
  }

  fclose(f);
  return 0;
}


/* int load_words_from_file(void) { */
/*   FILE* f = fopen("user_words.txt", "r"); */
/*   if (!f) { */
/*     perror("Could not open file for reading"); */
/*     return -1; */
/*   } */

/*   word_count = 0; */
/*   while (word_count < MAX_WORDS && !feof(f)) { */
/*     char name[MAX_WORD_NAME]; */
/*     char body[MAX_WORD_BODY]; */

/*     if (fscanf(f, "%31s %[^\n]", name, body) == 2) { */
/*       strncpy(words[word_count].name, name, MAX_WORD_NAME); */
/*       strncpy(words[word_count].body, body, MAX_WORD_BODY); */
/*       word_count++; */
/*     } */
/*   } */

/*   fclose(f); */
/*   return 0; */
/* } */

UserWord* find_word(const char* name) {
  for (int i = 0; i < word_count; i++) {
    if (strcmp(words[i].name, name) == 0) return &words[i];
  }
  return NULL;
}

