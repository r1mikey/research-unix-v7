struct	passwd { /* see getpwent(3) */
	char	*pw_name;
	char	*pw_passwd;
	short	pw_uid;
	short	pw_gid;
	short	pw_quota;
	char	*pw_comment;
	char	*pw_gecos;
	char	*pw_dir;
	char	*pw_shell;
};
