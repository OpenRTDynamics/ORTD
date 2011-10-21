mode(-1);
lines(0);

TOOLBOX_NAME = 'generic_libs_neu';
TOOLBOX_TITLE = 'generic libs module for Hart Toolbox';
// ====================================================================
buildable='yes'; //change to no, if the module should not be build


// Add the names of the c - funktions to names. E.g.: names=['rt_file1','rt_file2'];
names =   ['rt_ilc', 'rt_hart_libdyn', 'rt_filewriter'];

// Add the c file names which should be compiled to files. Replace the ending with .o. E.g.: files = ['file1.o';'file2.o'];
files     = [ 'hart_libdyn.cpp', 'filewriter_hart.cpp'];

// Add external shared librarys to ldflags
// E.g.: ldflags = " -lspezial_lib"; 
ldflags = " -lpthread -lortd "; 
cflags = " -g ";
