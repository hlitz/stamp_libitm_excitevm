# STAMP

This repository stores a version of STAMP that has been updated to comply
with the Draft C++ TM Specification, as implemented in GCC 4.7 and later.


## Folders

  * inputs/ -- This folder stores the inputs for KMeans, Yada, and Labyrinth
  * stamp-0.9.10/ -- This folder stores the original STAMP 0.9.10 source code, without any of the public patches
  * tests/ -- This folder stores tests done with vacation and bayes.  Please ignore it!
  * TRANSACT/ -- This folder contains a version of the code roughly equivalent to that presented at TRANSACT 2014.  Please use this version until the README indicates otherwise.

## Important Notes
  * To build with GCC on Linux, type "make" from the TRANSACT folder.  We
    haven't eliminated all warnings yet... please be patient :)

  * In Bayes, we uses a Taylor series to calculate a TM_SAFE logarithm.  This
    might not be optimal.  We also implemented our own TM_SAFE qsort() in
    shared lib/vector.  When testing Bayes, consider using a smaller
    workload, otherwise it may take too long a time.

  * In Genome, we have our own TM_SAFE strcmp and hash_sdbm. The former is a
    performance killer, causing "eager_tm" to run extremely slow. Replacing
    TM_SAFE strcmp with TM_PURE strcmp will restore the performance for
    eager_tm, but isn't really safe.
  
