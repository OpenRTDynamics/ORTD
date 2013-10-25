mode(-1);
lines(0);

TOOLBOX_NAME = 'OpenRTDynamics';
TOOLBOX_TITLE = 'OpenRTDynamics for Hart Toolbox';
// ====================================================================
buildable='yes'; //change to no, if the module should not be build


// Add the names of the c - funktions to names. E.g.: names=['rt_file1','rt_file2'];
names =   ['rt_ilc', 'rt_hart_libdyn', 'rt_filewriter'];

// Add the c file names which should be compiled to files. Replace the ending with .o. E.g.: files = ['file1.o';'file2.o'];
files     = [ 'ilc.o', 'hart_libdyn.o', 'filewriter_hart.o'];

// Add external shared librarys to ldflags
// E.g.: ldflags = " -lspezial_lib"; 
ldflags = " -lpthread -lortd "; 
cflags = "  ";
