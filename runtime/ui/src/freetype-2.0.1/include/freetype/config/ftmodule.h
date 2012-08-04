
// these are the modules necessary for antialiased
// true-type fonts.

FT_USE_MODULE(sfnt_module_class)
FT_USE_MODULE(ft_smooth_renderer_class)
FT_USE_MODULE(tt_driver_class)


// these modules can be included for other font types
// (you'll also need to add some extra .c files to
// your project file, and modify freetype.mk.  See the
// FreeType docs for more)

//FT_USE_MODULE(psnames_module_class)
//FT_USE_MODULE(autohint_module_class)
//FT_USE_MODULE(cff_driver_class)
//FT_USE_MODULE(t1cid_driver_class)
//FT_USE_MODULE(psaux_module_class)
//FT_USE_MODULE(ft_raster1_renderer_class)
//FT_USE_MODULE(t1_driver_class)
//FT_USE_MODULE(winfnt_driver_class)
