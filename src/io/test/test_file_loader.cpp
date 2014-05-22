/**
 * file_loader_test.cpp

 *
 *  Created on: Feb 18, 2014
 *      Author: tpan
 */


#include "config.hpp"    // for location of data.

#if defined(USE_MPI)
#include "mpi.h"
#endif

// include google test
#include <gtest/gtest.h>
#include <cstdint> // for uint64_t, etc.
#include <string>

#include "io/file_loader.hpp"

using namespace bliss::io;

template<typename Iterator, typename Range>
struct IdentityPartition {
    typedef typename Range::ValueType SizeType;
    typedef typename std::iterator_traits<Iterator>::value_type ValueType;
    typedef Iterator  IteratorType;

    const SizeType operator()(const Iterator &iter, const Range & parent, const Range &target) const {
      return std::min(parent.end, target.start + 2);
    }
};

template<typename Iter1, typename Iter2>
bool equal(const Iter1 &i1, const Iter2 &i2, size_t len, bool print = false) {
  Iter1 ii1(i1);
  Iter2 ii2(i2);
  for (size_t i = 0; i < len; ++i, ++ii1, ++ii2) {
    if (print && *ii1 != *ii2) std::cout << i << " ";

    if (*ii1 != *ii2) return false;
  }
  return true;
}


template <typename T>
class FileLoaderTest : public ::testing::Test
{
  protected:
    virtual void SetUp()
    {
      fileName.assign(PROJ_SRC_DIR);
      fileName.append("/test/data/test.fastq");

      // get file size
      struct stat filestat;
      stat(fileName.c_str(), &filestat);
      fileSize = static_cast<size_t>(filestat.st_size);

      ASSERT_EQ(34111308, fileSize);

    }

    static void readFilePOSIX(const std::string &fileName, const size_t& offset,
                              const size_t& length, T* result)
    {
      FILE *fp = fopen(fileName.c_str(), "r");
      fseek(fp, offset * sizeof(T), SEEK_SET);
      size_t read = fread_unlocked(result, sizeof(T), length, fp);
      fclose(fp);

      ASSERT_GT(read, 0);
    }

    std::string fileName;
    size_t fileSize;      // in units of T.

};

// indicate this is a typed test
TYPED_TEST_CASE_P(FileLoaderTest);


// normal test cases
TYPED_TEST_P(FileLoaderTest, OpenWithFullRange)
{
  typedef FileLoader<TypeParam, false, false> FileLoaderType;
  typedef typename FileLoaderType::RangeType RangeType;

  // get fileName

  FileLoaderType loader(this->fileName);
  RangeType r = loader.getRange();

  size_t len = r.end - r.start;
  TypeParam* gold = new TypeParam[len + 1];
  FileLoaderTest<TypeParam>::readFilePOSIX(this->fileName, r.start, len, gold);

  loader.load();
  int comp = memcmp(gold, loader.getData().begin(), len * sizeof(TypeParam));
  ASSERT_EQ(0, comp);
  delete [] gold;
  loader.unload();
}

// normal test cases
TYPED_TEST_P(FileLoaderTest, PreloadWithFullRange)
{
  typedef FileLoader<TypeParam, false, true> FileLoaderType;
  typedef typename FileLoaderType::RangeType RangeType;

  // get fileName

  FileLoaderType loader(this->fileName);
  RangeType r = loader.getRange();

  size_t len = r.end - r.start;
  TypeParam* gold = new TypeParam[len + 1];
  FileLoaderTest<TypeParam>::readFilePOSIX(this->fileName, r.start, len, gold);

  loader.load();
  ASSERT_TRUE(equal(gold, loader.getData().begin(), len));
  delete [] gold;
  loader.unload();

}


TYPED_TEST_P(FileLoaderTest, OpenWithRange)
{
  typedef FileLoader<TypeParam, false, false> FileLoaderType;
  typedef typename FileLoaderType::RangeType RangeType;

  // get this->fileName
  int rank = 3;
  int nprocs = 7;

  FileLoaderType loader(this->fileName);

  RangeType r = loader.getFileRange().block_partition(nprocs, rank);
  loader.setRange(r);

  size_t len = r.end - r.start;
  TypeParam* gold = new TypeParam[len + 1];
  FileLoaderTest<TypeParam>::readFilePOSIX(this->fileName, r.start, len, gold);

  loader.load();
  int comp = memcmp(gold, loader.getData().begin(), len * sizeof(TypeParam));
  ASSERT_EQ(0, comp);
  delete [] gold;
  loader.unload();
}

TYPED_TEST_P(FileLoaderTest, OpenWithAlignedRange)
{
  typedef FileLoader<TypeParam, false, false> FileLoaderType;
  typedef typename FileLoaderType::RangeType RangeType;

  // get this->fileName
  int rank = 3;
  int nprocs = 7;


  FileLoaderType loader(this->fileName);

  RangeType r = loader.getFileRange().block_partition(nprocs, rank);
  RangeType ra = r.align_to_page(sysconf(_SC_PAGE_SIZE));

  loader.setRange(ra);
  loader.load();

  size_t len = ra.end - ra.start;
  TypeParam* gold = new TypeParam[len + 1];
  FileLoaderTest<TypeParam>::readFilePOSIX(this->fileName, ra.start, len, gold);
  int comp = memcmp(gold, loader.getData().begin(), len * sizeof(TypeParam));
  ASSERT_EQ(0, comp);

  len = r.end - r.start;
  FileLoaderTest<TypeParam>::readFilePOSIX(this->fileName, r.start, len, gold);
  comp = memcmp(gold, loader.getData().begin(), len * sizeof(TypeParam));
  ASSERT_EQ(0, comp);


  delete [] gold;
  loader.unload();
}


// normal test cases
TYPED_TEST_P(FileLoaderTest, PreloadWithRange)
{
  typedef FileLoader<TypeParam, false, true> FileLoaderType;
  typedef typename FileLoaderType::RangeType RangeType;

  // get this->fileName
  int rank = 3;
  int nprocs = 7;

  FileLoaderType loader(this->fileName);

  RangeType r = loader.getFileRange().block_partition(nprocs, rank);
  loader.setRange(r);

  size_t len = r.end - r.start;
  TypeParam* gold = new TypeParam[len + 1];
  FileLoaderTest<TypeParam>::readFilePOSIX(this->fileName, r.start, len, gold);

  loader.load();
  ASSERT_TRUE(equal(gold, loader.getData().begin(), len));
  delete [] gold;
  loader.unload();
}


TYPED_TEST_P(FileLoaderTest, OpenConsecutiveRanges)
{
  typedef FileLoader<TypeParam, false, false> FileLoaderType;
  typedef typename FileLoaderType::RangeType RangeType;

  // get this->fileName
  int nprocs = 7;

  FileLoaderType loader(this->fileName);
  RangeType r;

  for (int rank = 0; rank < nprocs; ++rank) {

    r = loader.getFileRange().block_partition(nprocs, rank);
    loader.setRange(r);

    size_t len = r.end - r.start;
    TypeParam* gold = new TypeParam[len + 1];
    FileLoaderTest<TypeParam>::readFilePOSIX(this->fileName, r.start, len, gold);

    loader.load();
    int comp = memcmp(gold, loader.getData().begin(), len * sizeof(TypeParam));
    ASSERT_EQ(0, comp);
    delete [] gold;
    loader.unload();
  }
}

// normal test cases
TYPED_TEST_P(FileLoaderTest, AdjustRange)
{
  typedef FileLoader<TypeParam, false, false> FileLoaderType;
  typedef typename FileLoaderType::RangeType RangeType;

  // get this->fileName
  int rank = 3;
  int nprocs = 7;

  FileLoaderType loader(this->fileName);

  RangeType r = loader.getFileRange().block_partition(nprocs, rank);
  loader.setRange(r);
  loader.adjustRange(IdentityPartition<typename FileLoaderType::InputIteratorType, RangeType>());
  RangeType r2 = loader.getRange();

  ASSERT_EQ(r.start + 2, r2.start);
}


TYPED_TEST_P(FileLoaderTest, AdjustConsecutiveRanges)
{
  typedef FileLoader<TypeParam, false, false> FileLoaderType;
  typedef typename FileLoaderType::RangeType RangeType;

  // get this->fileName
  int nprocs = 7;
  typename RangeType::ValueType lastEnd = 2;

  FileLoaderType loader(this->fileName);
  RangeType r, r2;
  IdentityPartition<typename FileLoaderType::InputIteratorType, RangeType> ip;

  for (int rank = 0; rank < nprocs; ++rank) {

    r = loader.getFileRange().block_partition(nprocs, rank);
    loader.setRange(r);
    loader.adjustRange(ip);
    r2 = loader.getRange();

    ASSERT_EQ(lastEnd, r2.start);
    lastEnd = r2.end;
  }
}

// normal test cases
TYPED_TEST_P(FileLoaderTest, BufferChunks)
{
  typedef FileLoader<TypeParam, true, false> FileLoaderType;
  typedef typename FileLoaderType::RangeType RangeType;

  // get this->fileName
  int rank = 3;
  int nprocs = 7;


  FileLoaderType loader(this->fileName);

  RangeType r = loader.getFileRange().block_partition(nprocs, rank);
  loader.setRange(r);
  loader.load();
  typename RangeType::ValueType lastEnd = r.start + 2;

  RangeType r2;
  size_t len;
  TypeParam* gold;

  IdentityPartition<typename FileLoaderType::IteratorType, RangeType> ip;
  typename FileLoaderType::DataBlockType data = loader.getNextChunkAtomic(ip, 2048);

  r2 = data.getRange();
  len = r2.end - r2.start;

  while (len  > 0) {

    ASSERT_EQ(lastEnd, r2.start);
    lastEnd = r2.end;

    gold = new TypeParam[len + 1];
    FileLoaderTest<TypeParam>::readFilePOSIX(this->fileName, r2.start, len, gold);

    ASSERT_TRUE(equal(gold, data.begin(), len));
    delete [] gold;

    data= loader.getNextChunkAtomic(ip, 2048);
    r2 = data.getRange();
    len = r2.end - r2.start;
  }

  loader.unload();
}

// normal test cases
TYPED_TEST_P(FileLoaderTest, UnbufferChunks)
{
  typedef FileLoader<TypeParam, false, false> FileLoaderType;
  typedef typename FileLoaderType::RangeType RangeType;

  // get this->fileName
  int rank = 3;
  int nprocs = 7;


  FileLoaderType loader(this->fileName);

  RangeType r = loader.getFileRange().block_partition(nprocs, rank);
  loader.setRange(r);
  loader.load();

  RangeType r2;
  size_t len;
  TypeParam* gold;
  typename RangeType::ValueType lastEnd = r.start + 2;


  IdentityPartition<typename FileLoaderType::IteratorType, RangeType> ip;
  typename FileLoaderType::DataBlockType data = loader.getNextChunkAtomic(ip, 2048);


  r2 = data.getRange();
  len = r2.end - r2.start;

  while (len  > 0) {

    EXPECT_NE(lastEnd, r2.end);
    ASSERT_EQ(lastEnd, r2.start);
    lastEnd = r2.end;

    gold = new TypeParam[len + 1];
    FileLoaderTest<TypeParam>::readFilePOSIX(this->fileName, r2.start, len, gold);

    ASSERT_TRUE(equal(gold, data.begin(), len));
    delete [] gold;

    data= loader.getNextChunkAtomic(ip, 2048);
    r2 = data.getRange();
    len = r2.end - r2.start;
  }

  loader.unload();
}

// normal test cases
TYPED_TEST_P(FileLoaderTest, BufferChunksWithPreload)
{
  typedef FileLoader<TypeParam, true, true> FileLoaderType;
  typedef typename FileLoaderType::RangeType RangeType;

  // get this->fileName
  int rank = 3;
  int nprocs = 7;


  FileLoaderType loader(this->fileName);

  RangeType r = loader.getFileRange().block_partition(nprocs, rank);
  loader.setRange(r);
  loader.load();
  typename RangeType::ValueType lastEnd = r.start + 2;

  RangeType r2;
  size_t len;
  TypeParam* gold;

  IdentityPartition<typename FileLoaderType::IteratorType, RangeType> ip;
  typename FileLoaderType::DataBlockType data = loader.getNextChunkAtomic(ip, 2048);


  r2 = data.getRange();
  len = r2.end - r2.start;

  while (len  > 0) {

    ASSERT_EQ(lastEnd, r2.start);
    lastEnd = r2.end;

    gold = new TypeParam[len + 1];
    FileLoaderTest<TypeParam>::readFilePOSIX(this->fileName, r2.start, len, gold);

    ASSERT_TRUE(equal(gold, data.begin(), len));
    delete [] gold;

    data= loader.getNextChunkAtomic(ip, 2048);
    r2 = data.getRange();
    len = r2.end - r2.start;
  }

  loader.unload();
}

// normal test cases
TYPED_TEST_P(FileLoaderTest, UnbufferChunksWithPreload)
{
  typedef FileLoader<TypeParam, false, true> FileLoaderType;
  typedef typename FileLoaderType::RangeType RangeType;

  // get this->fileName
  int rank = 3;
  int nprocs = 7;


  FileLoaderType loader(this->fileName);

  RangeType r = loader.getFileRange().block_partition(nprocs, rank);
  loader.setRange(r);
  loader.load();

  RangeType r2;
  size_t len;
  TypeParam* gold;
  typename RangeType::ValueType lastEnd = r.start + 2;

  IdentityPartition<typename FileLoaderType::IteratorType, RangeType> ip;
  typename FileLoaderType::DataBlockType data = loader.getNextChunkAtomic(ip, 2048);


  r2 = data.getRange();
  len = r2.end - r2.start;

  int i = 0;

  while (len > 0) {

    ASSERT_EQ(lastEnd, r2.start);
    lastEnd = r2.end;

    gold = new TypeParam[len + 1];
    FileLoaderTest<TypeParam>::readFilePOSIX(this->fileName, r2.start, len, gold);
    gold[len] = 0;

    ASSERT_TRUE(equal(gold, data.begin(), len));
    delete [] gold;

    data= loader.getNextChunkAtomic(ip, 2048);
    r2 = data.getRange();
    len = r2.end - r2.start;

    ++i;
  }

  loader.unload();
}


// now register the test cases
REGISTER_TYPED_TEST_CASE_P(FileLoaderTest, OpenWithFullRange, PreloadWithFullRange,
                           OpenWithRange, OpenWithAlignedRange, PreloadWithRange, OpenConsecutiveRanges, AdjustRange, AdjustConsecutiveRanges,
                           BufferChunks, UnbufferChunks, BufferChunksWithPreload, UnbufferChunksWithPreload);


//template <typename T>
//class FileLoaderDeathTest : public ::testing::Test
//{
//  protected:
//    virtual void SetUp()
//    {
//    }
//
//};
//
//// indicate this is a typed test
//TYPED_TEST_CASE_P(FileLoaderDeathTest);
//
//
//// TODO negative test cases
//TYPED_TEST_P(FileLoaderDeathTest, NoFilename)
//{
//  typedef file_loader<TypeParam> FileLoaderType;
//
//  std::string fileName;
//
//  std::string err_regex = ".*file_loader.hpp.* Assertion .* failed.*";
//  EXPECT_EXIT(FileLoaderType(fileName), ::testing::KilledBySignal(SIGABRT), err_regex);
//
//}
//
//TYPED_TEST_P(FileLoaderDeathTest, BadFilename)
//{
//  typedef file_loader<TypeParam> FileLoaderType;
//
//  std::string fileName;
//  fileName.assign("bad");
//
//  std::string err_regex = ".*file_loader.hpp.* Exception .* ERROR .* error 2: No such file or directory";
//  EXPECT_THROW(FileLoaderType(fileName), bliss::io::io_exception);
//}
//
//
//// now register the test cases
//REGISTER_TYPED_TEST_CASE_P(FileLoaderDeathTest, NoFilename, BadFilename);




typedef ::testing::Types<unsigned char, char> //, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t>
    FileLoaderTestTypes;
INSTANTIATE_TYPED_TEST_CASE_P(Bliss, FileLoaderTest, FileLoaderTestTypes);
//INSTANTIATE_TYPED_TEST_CASE_P(Bliss, FileLoaderDeathTest, FileLoaderTestTypes);



