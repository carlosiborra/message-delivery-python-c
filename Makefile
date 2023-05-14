CPPFLAGS = -I$(INSTALL_PATH)/include -Wall -Wextra -Werror

LDFLAGS = -L$(INSTALL_PATH)/lib/

LDLIBS = -lpthread

# Adding ./lib directory to the LD_LIBRARY_PATH environment variable and exporting it
LD_LIBRARY_PATH = $LD_LIBRARY_PATH:./lib
export LD_LIBRARY_PATH

# check if hostname == guernika using a function called get_compiler
get_compiler = $(if $(findstring guernika,$1),/opt/gcc-12.1.0/bin/gcc,gcc)

# Default target
all: proxy

# Print output files information 
# information:
# 	@echo ''
# 	@echo "Output files:"
# 	@echo "  - servidor"
# 	@echo ''

# # proxy.c compilation
# proxy: proxy.c lines.c servidor.c LinkedList.c
# 	@$(call get_compiler, $(shell hostname)) $(LDFLAGS) $(CPPFLAGS) $(LDLIBS) $^ -o servidor

# proxy.c compilation
proxy: lines.c proxy.c servidor.c LinkedList.c
	@$(call get_compiler, $(shell hostname)) $(LDFLAGS) $(CPPFLAGS) $(LDLIBS) $^ -o servidor

# Clean all files
clean:
	@rm -f *.o *.out *.so ./lib/*.so -d ./lib cliente servidor
	@echo -e '\n'"All files removed"'\n'
