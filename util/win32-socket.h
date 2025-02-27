/*
  Copyright (C) 2022 Ángel Ruiz Fernández

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, version 3.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program.  If not, see
  <http://www.gnu.org/licenses/>
*/

#ifndef _UTIL_SOCKET_H
#define _UTIL_SOCKET_H

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>

#define getsockopt(sockfd, level, optname, optval, optLen) \
  getsockopt(sockfd, level, optname, (char *)optval, optLen)
#define setsockopt(sockfd, level, optname, optval, optLen) \
  setsockopt(sockfd, level, optname, (const char *)optval, optLen)
#define send(sockfd, buf, len, flags) \
  send(sockfd, (const char *)buf, len, flags)

#ifdef interface
#  undef interface
#endif /* interface */

#endif /* _UTIL_SOCKET_H */