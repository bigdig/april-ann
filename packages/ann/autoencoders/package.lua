 package{ name = "ann.autoencoders",
   version = "1.0",
   depends = { "ann", "dataset", "matrix", "util", "trainable" },
   keywords = { "Stacked Denoising Auto-Encoders" },
   description = "Implements a LUA function for training Stacked Denoising Auto-Encoers",
   -- targets como en ant
   target{
     name = "init",
     mkdir{ dir = "build" },
     mkdir{ dir = "include" },
   },
   target{ name = "clean",
     delete{ dir = "build" },
     delete{ dir = "include" },
   },
   target{
     name = "test",
     lua_unit_test{ file="test/test.lua" },
     lua_unit_test{ file="test/test_on_the_fly.lua" },
   },
   target{
     name = "provide",
     depends = "init",
     -- copy{ file= "c_src/*.h", dest_dir = "include" },
     -- provide_bind{ file = "binding/bind_mlp.lua.cc", dest_dir = "include" }
   },
   target{
     name = "build",
     depends = "provide",
     use_timestamp = true,
     -- object{ 
     --  file = "c_src/*.cc",
     --  include_dirs = "${include_dirs}",
     --  dest_dir = "build",
     -- },
     luac{
       orig_dir = "lua_src",
       dest_dir = "build",
     },
     -- build_bind{
     --   file = "binding/bind_mlp.lua.cc",
     --   dest_dir = "build",
     -- }
   },
   target{
     name = "document",
     document_src{
     },
     document_bind{
     },
   },
 }
 
 
