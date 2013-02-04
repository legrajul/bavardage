
user *find_user (char *login, userlist *users) {
	user *cur_us, *cur_pred;
	cur_us = users;
	while (cur_us != NULL) {
		if (strcmp(cur_us->user.name, login) == 1) {
			printf("%s found\n", s);
			strcpy(cur_us.name, s);
			return cur_pred;
		}
		cur_pred = cur_us;
		cur_us = cur_us->next;
	}
	
	if (cur_us == NULL) {
		printf("%s is not present\n", login);
		return NULL;
	}
}


int remove_user_list (char *login, userlist *users) {
	user *us = find_user(login, users);
	user *fr;
	if (us == NULL) {
		perror("User does not exist");
		return -1;
	} else {
		if (us->next->next == NULL) {
			//~ us->next = NULL;
			free(us);
		} else {
			fr = us->next;
			us->next = us->next->next;
			free(fr);
		}
	}
	return 1;
}

int delete_user(char *login, user_map *map) {
	int h = hash(login);
	int i;
	for (i=0; i < HASH_USER_MAP_SIZE; i++) {
		if (i == h)
			remove_user_list(login, map[i]);
	}
	return 1;
}
