/*
 * Structure of a symbol table entry
 */

struct	symbol {
	char	sy_name[8];
	unsigned short	sy_type;
	unsigned short	sy_pad;
	unsigned int	sy_value;
};
