

function ORTD_install()
    path = uigetdir();

    ok = %F;

try
    printf("Try to locate " + path + '/THIS_IS_ORTD ...\n');
    fd=mopen(path + '/THIS_IS_ORTD', 'rt');
    tmp=mgetl(fd, 1);
    mclose(fd);
    ok=%T;    
catch
  printf("\nNOTE: ORTD was not found within this directory\n\n");  
end



if ok==%t then
    try
    fd=mopen(ORTD.scilabtoolboxpath + 'etc/ortd_path.conf', 'wt');
    mputl(path, fd);
    mclose(fd);
    printf("ORTD was found and the scilab toolbox was configured.\n");
catch
  printf("There was an error writing the configuration file. Maybe there is no permission for writing?\n");
end



end



endfunction


//function ()
//endfunction
try
    fd=mopen(ORTD.scilabtoolboxpath + 'etc/ortd_path.conf', 'rt');
    ORTD.ortdpath=mgetl(fd, 1);
    mclose(fd);
    
    ORTD.ortd_executable = ORTD.ortdpath + "/bin/libdyn_generic_exec_static_scilab";
catch
  printf("\nNOTE: The OpenRTDynamics main directory was not found. Tell me the path using ORTD_install()\n\n");  
  ORTD.ortd_executable = "libdyn_generic_exec_scilab ";  
end
