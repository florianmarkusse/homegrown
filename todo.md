- Move basic modules from x/a/a.h to x/a.h
- Make more includes private/interface wherever possible
- Check that the build does not contain duplicate functions
- lto stuff
- Start rewrite of image-builder

- Is there a way to only build the libraries a project relies on instead of all the libraries in a certain project?
  - Do after the transition to object libraries
  - Can you somehow store the .o files to be reused by another project that uses the same flags?
