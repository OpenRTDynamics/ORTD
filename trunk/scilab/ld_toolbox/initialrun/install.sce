

function ORTD_install()
    path = uigetdir();
//
//    u=file('open', ORTD.scilabtoolboxpath + 'etc/ortd_path.conf','unknown');
//    fprintf(u, path);
//
//    file('close',u);
//    
//    
    fd=mopen(ORTD.scilabtoolboxpath + 'etc/ortd_path.conf', 'wt');
    mputl(path, fd);
    mclose(fd);
endfunction


//function ()
//endfunction
try
    fd=mopen(ORTD.scilabtoolboxpath + 'etc/ortd_path.conf', 'rt');
    ORTD.ortdpath=mgetl(fd, 1);
    mclose(fd);
    
    ORTD.ortd_executable = ORTD.ortdpath + "/bin/libdyn_generic_exec_static ";
catch
  printf("\nNOTE: The OpenRTDynamics main directory was not found. Tell me the path using ORTD_install()\n\n");  
  ORTD.ortd_executable = "libdyn_generic_exec ";  
end
