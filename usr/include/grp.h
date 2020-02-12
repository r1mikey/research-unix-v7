struct	group { /* see getgrent(3) */
	char	*gr_name;
	char	*gr_passwd;
	short	gr_gid;
	char	**gr_mem;
};
