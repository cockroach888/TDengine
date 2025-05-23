/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "tsdbDef.h"
#include "tsdbFSet2.h"
#include "tsdbSttFileRW.h"
#include "tsdbUtil2.h"

#ifndef _TSDB_DATA_FILE_RW_H
#define _TSDB_DATA_FILE_RW_H

#ifdef __cplusplus
extern "C" {
#endif

typedef TARRAY2(SColumnDataAgg) TColumnDataAggArray;

typedef struct {
  SFDataPtr brinBlkPtr[1];
  char      rsrvd[32];
} SHeadFooter;

typedef struct {
  SFDataPtr tombBlkPtr[1];
  char      rsrvd[32];
} STombFooter;

// SDataFileReader =============================================
typedef struct SDataFileReader SDataFileReader;
typedef struct SDataFileReaderConfig {
  STsdb  *tsdb;
  int32_t szPage;
  struct {
    bool   exist;
    STFile file;
  } files[TSDB_FTYPE_MAX];
  SBuffer *buffers;
} SDataFileReaderConfig;

int32_t tsdbDataFileReaderOpen(const char *fname[/* TSDB_FTYPE_MAX */], const SDataFileReaderConfig *config,
                               SDataFileReader **reader);
void    tsdbDataFileReaderClose(SDataFileReader **reader);
// .head
int32_t tsdbDataFileReadBrinBlk(SDataFileReader *reader, const TBrinBlkArray **brinBlkArray);
int32_t tsdbDataFileReadBrinBlock(SDataFileReader *reader, const SBrinBlk *brinBlk, SBrinBlock *brinBlock);
// .data
int32_t tsdbDataFileReadBlockData(SDataFileReader *reader, const SBrinRecord *record, SBlockData *bData);
int32_t tsdbDataFileReadBlockDataByColumn(SDataFileReader *reader, const SBrinRecord *record, SBlockData *bData,
                                          STSchema *pTSchema, int16_t cids[], int32_t ncid);
// .sma
int32_t tsdbDataFileReadBlockSma(SDataFileReader *reader, const SBrinRecord *record,
                                 TColumnDataAggArray *columnDataAggArray);
// .tomb
int32_t tsdbDataFileReadTombBlk(SDataFileReader *reader, const TTombBlkArray **tombBlkArray);
int32_t tsdbDataFileReadTombBlock(SDataFileReader *reader, const STombBlk *tombBlk, STombBlock *tData);

// SDataFileWriter =============================================
typedef struct SDataFileWriter SDataFileWriter;
typedef struct SDataFileWriterConfig {
  STsdb   *tsdb;
  uint32_t cmprAlg;
  int32_t  maxRow;
  int32_t  szPage;
  int32_t  fid;
  int64_t  cid;
  int32_t  expLevel;
  int64_t  compactVersion;
  int32_t  lcn;
  struct {
    bool   exist;
    STFile file;
  } files[TSDB_FTYPE_MAX];
  SSkmInfo *skmTb;
  SSkmInfo *skmRow;
  SHashObj *pColCmpr;
  SBuffer  *buffers;
} SDataFileWriterConfig;

int32_t tsdbDataFileWriterOpen(const SDataFileWriterConfig *config, SDataFileWriter **writer);
int32_t tsdbDataFileWriterClose(SDataFileWriter **writer, bool abort, TFileOpArray *opArr);

int32_t tsdbDataFileWriteRow(SDataFileWriter *writer, SRowInfo *row);
int32_t tsdbDataFileWriteBlockData(SDataFileWriter *writer, SBlockData *bData);
int32_t tsdbDataFileFlush(SDataFileWriter *writer);

// head
int32_t tsdbFileWriteBrinBlock(STsdbFD *fd, SBrinBlock *brinBlock, uint32_t cmprAlg, int64_t *fileSize,
                               TBrinBlkArray *brinBlkArray, SBuffer *buffers, SVersionRange *range,
                               int32_t encryptAlgorithm, char *encryptKey);
int32_t tsdbFileWriteBrinBlk(STsdbFD *fd, TBrinBlkArray *brinBlkArray, SFDataPtr *ptr, int64_t *fileSize,
                             int32_t encryptAlgorithm, char *encryptKey);
int32_t tsdbFileWriteHeadFooter(STsdbFD *fd, int64_t *fileSize, const SHeadFooter *footer, int32_t encryptAlgorithm,
                                char *encryptKey);

// tomb
int32_t tsdbDataFileWriteTombRecord(SDataFileWriter *writer, const STombRecord *record);
int32_t tsdbFileWriteTombBlock(STsdbFD *fd, STombBlock *tombBlock, int8_t cmprAlg, int64_t *fileSize,
                               TTombBlkArray *tombBlkArray, SBuffer *buffers, SVersionRange *range,
                               int32_t encryptAlgorithm, char *encryptKey);
int32_t tsdbFileWriteTombBlk(STsdbFD *fd, const TTombBlkArray *tombBlkArray, SFDataPtr *ptr, int64_t *fileSize,
                             int32_t encryptAlgorithm, char *encryptKey);
int32_t tsdbFileWriteTombFooter(STsdbFD *fd, const STombFooter *footer, int64_t *fileSize, int32_t encryptAlgorithm,
                                char *encryptKey);

// utils
void tsdbWriterUpdVerRange(SVersionRange *range, int64_t minVer, int64_t maxVer);
void tsdbTFileUpdVerRange(STFile *f, SVersionRange range);

#ifdef __cplusplus
}
#endif

#endif /*_TSDB_DATA_FILE_RW_H*/
