
demopath = get_absolute_file_path("ld_toolbox.dem.gateway.sce");

subdemolist=['Introduction', 'intro_demo.sce'];


subdemolist(:,2) = demopath + subdemolist(:,2);
// ====================================================================
