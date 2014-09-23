/*
 * PLEASE SEE LICENSE FILE FOR LICENSING AND COPYRIGHT INFORMATION
 */

/*
 * segments.h: Create random segments from random gene
 */

#pragma once

#include "gene.h"
#include "vector.h"

struct segments_t {
    long length;
    long minNum;
    vector_t* contentsPtr;
  private:
    char** strings;
  public:
  /*
   * constructor
   * -- Does almost all the memory allocation for random segments
   * -- The actual number of segments created by 'segments_create' may be larger
   *    than 'minNum' to ensure the segments overlap and cover the entire gene
   */
  segments_t(long length, long minNum);

  /*
   * create(): Populates 'contentsPtr'
   */
  void create(gene_t* genePtr, std::mt19937* randomPtr);

  /*
   * destructor
   */
  ~segments_t();
};
