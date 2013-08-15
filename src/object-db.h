/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2012 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 *	   Zhenkai Zhu <zhenkai@cs.ucla.edu>
 */

#ifndef OBJECT_DB_H
#define OBJECT_DB_H

#include <string>
#include <sqlite3.h>
#include <ndnx-common.h>
#include <ndnx-name.h>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <ctime>
#include <vector>

class ObjectDb
{
public:
  // database will be create in <folder>/<first-pair-of-hash-bytes>/<rest-of-hash>
  ObjectDb (const boost::filesystem::path &folder, const std::string &hash);
  ~ObjectDb ();

  void
  saveContentObject (const Ndnx::Name &deviceName, sqlite3_int64 segment, const Ndnx::Bytes &data);

  Ndnx::BytesPtr
  fetchSegment (const Ndnx::Name &deviceName, sqlite3_int64 segment);

  // sqlite3_int64
  // getNumberOfSegments (const Ndnx::Name &deviceName);

  time_t
  secondsSinceLastUse();

  static bool
  DoesExist (const boost::filesystem::path &folder, const Ndnx::Name &deviceName, const std::string &hash);

private:
  void
  willStartSave ();

  void
  didStopSave ();

private:
  sqlite3 *m_db;
  time_t m_lastUsed;
};

typedef boost::shared_ptr<ObjectDb> ObjectDbPtr;

#endif // OBJECT_DB_H
