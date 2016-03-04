#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct tree_node;
struct list_node;
struct tree_node *make_tree_node(char *name);
struct list_node *make_list_node(struct tree_node *v, struct list_node *next);
void do_mkdir(struct tree_node *cwd, char *arg);
void do_ls(struct tree_node *cwd);
struct tree_node *do_cd(struct tree_node *cwd, struct tree_node *root,
                        char *arg);
void show_prompt(struct tree_node *cwd);
void remove_child(struct tree_node *dir, struct tree_node *child);
void do_rmdir(struct tree_node *cwd, char *arg);
void free_up(struct tree_node *root);

struct tree_node {
  char buffer[128];
  struct tree_node *parent;
  struct list_node *firstChild;
};

struct list_node {
  struct tree_node *represents;
  struct list_node *next;
};

struct tree_node *make_tree_node(char *name) {
  struct tree_node *treeNode =
      malloc(sizeof(struct tree_node)); // Allocate Memory
  treeNode->firstChild = NULL;
  treeNode->parent = NULL;
  strcpy(treeNode->buffer, name);
  return treeNode;
}

struct list_node *make_list_node(struct tree_node *v, struct list_node *next) {
  struct list_node *listNode =
      malloc(sizeof(struct list_node)); // Allocate Memory
  listNode->represents = v;
  listNode->next = next;
  return listNode;
}

void do_mkdir(struct tree_node *cwd, char *arg) {
  struct list_node *temp = cwd->firstChild;
  if (arg == NULL || strcmp(arg, "") == 0) { // Checks if nothing entered
    printf("Error! No argument entered!\n");
    return;
  } else if (isspace(*arg)) {
    printf("No whitespaces allowed!!\n");
    return;
  }

  if (cwd->firstChild == NULL) {
    cwd->firstChild =
        make_list_node(make_tree_node(arg),
                       NULL); // Directly makes a node if firstchild is null.
    cwd->firstChild->represents->parent = cwd;
    printf("Directory created\n");
    return;
  }
  if (strcmp(temp->represents->buffer, arg) ==
      0) { // Checks if the directory already exists at start
    printf("Error! Directory '%s' already exists!\n", temp->represents->buffer);
    return;
  }
  while (temp->next != NULL) {
    if (strcmp(temp->next->represents->buffer, arg) ==
        0) { // Checks the rest of the buffer to see if name exists.
      printf("Error! Directory '%s' already exists!\n",
             temp->next->represents->buffer);
      return;
    }
    temp = temp->next;
  }
  temp->next = make_list_node(make_tree_node(arg), NULL);
  temp->next->represents->parent = cwd;
  printf("Directory created\n");
}

void do_ls(struct tree_node *cwd) {
  struct list_node *children = cwd->firstChild;
  if (children == NULL) {
    printf("No directories found!\n");
    return;
  }
  while (children != NULL) {
    printf("-  %s\n", children->represents->buffer);
    children = children->next;
  }
}

struct tree_node *do_cd(struct tree_node *cwd, struct tree_node *root,
                        char *arg) {

  if (arg == NULL || strcmp(arg, "") == 0 || strcmp(arg, "..") == 0 ||
      isspace(*arg)) {
    if (!cwd->parent) {
      return cwd;
    } else {
      return cwd->parent;
    }
  } else {
    struct list_node *tempNode = cwd->firstChild;
    while (tempNode != NULL) {
      if (strcmp(tempNode->represents->buffer, arg) == 0) {
        return tempNode->represents;
      }
      tempNode = tempNode->next;
    }
    printf("Directory doesn't exist!\n");
    return cwd;
  }
}

void show_prompt(struct tree_node *cwd) {
  if (cwd != NULL) {
    show_prompt(
        cwd->parent); // Recursive function that keeps dispaying new directory.
    printf("%s/", cwd->buffer);
  }
}

void remove_child(struct tree_node *dir, struct tree_node *child) {
  struct list_node *temp = dir->firstChild;
  if (strcmp(temp->represents->buffer, child->buffer) == 0) {
    dir->firstChild = temp->next; // Makes the first child the temp->next node
    free(temp->represents);
    free(temp);
    return;
  } else {
    while (temp->next != NULL) {
      if (strcmp(temp->next->represents->buffer, child->buffer) == 0) {
        struct list_node *tempTwo = temp->next;
        temp->next = temp->next->next; // Points to the next node
        free(tempTwo->represents);     // Frees the node
        free(tempTwo);
        return;
      }
      temp = temp->next;
    }
  }
}

void do_rmdir(struct tree_node *cwd, char *arg) {
  struct list_node *temp = cwd->firstChild;
  if (arg == NULL) {
    printf("Please enter an argument!\n"); // Checks if its NULL
  } else if (strcmp(arg, "") == 0) {
    printf("Please enter an argument!\n"); // Checks if there is no character
                                           // entered
  } else if (isspace(*arg)) {
    printf("No whitespaces allowed!\n"); // Checks if a space character has been
                                         // entered
  } else {
    while (temp != NULL) {
      if (strcmp(temp->represents->buffer, arg) == 0) {
        if (temp->represents->firstChild != NULL) {
          printf("Not null!\n");
          return;
        } else {
          remove_child(cwd, temp->represents);
          printf("Directory Removed!\n");
          return;
        }
      }
      temp = temp->next;
    }
  }
  printf("Error! Directory doesn't exist!\n");
}

void free_up(struct tree_node *root) {
  struct list_node *temp = root->firstChild;
  struct list_node *tempTwo = NULL;
  while (temp != NULL) { // Keeps looping while there are children
    if (temp->represents->firstChild != NULL) {
      free_up(temp->represents); // Recursive function to free all children
    }
    tempTwo = temp;
    temp = temp->next;
    free(tempTwo->represents); // Frees tempTwo node
    free(tempTwo);             // Frees temp
  }
}

int main() {
  struct tree_node *root = make_tree_node("root");
  struct tree_node *cwd = root;

  printf("######### FILE SYSTEM #########\n");
  char input[134];
  int isRunning = 1;

  while (isRunning == 1) {
    show_prompt(cwd);
    printf(" > ");

    fgets(input, sizeof(input), stdin);

    char *command = strtok(input, " "); // separates by spaces
    char *argument = strtok(NULL, " "); // separates by spaces

    strtok(command, "\n");  // removes new line character
    strtok(argument, "\n"); // removes new line character

    if (strcmp(command, "ls") == 0) {
      do_ls(cwd);
    } else if (strcmp(command, "cd") == 0) {
      cwd = do_cd(cwd, root, argument);
    } else if (strcmp(command, "mkdir") == 0) {
      do_mkdir(cwd, argument);
    } else if (strcmp(command, "rmdir") == 0) {
      do_rmdir(cwd, argument);
    } else if (strcmp(command, "clear") == 0) {
      system("clear"); // Simulates clear screen
    } else if (strcmp(command, "exit") == 0) {
      printf("Goodbye!\n");
      free_up(root); // Frees memory if nodes arnt removed in program
      free(root);    // Frees up root
      isRunning = 0;
    } else {
      printf("Not a valid command!\n");
    }
  }
  return 0;
}
