PHP_ARG_ENABLE(phalcon, whether to enable phalcon7 framework,
[  --enable-phalcon        Enable phalcon7 framework])

PHP_ARG_ENABLE(phalcon-debug, for phalcon7 debug support,
[  --enable-phalcon-debug  Enable enable phalcon7 debug support], no, no)

if test "$PHP_PHALCON_DEBUG" != "no"; then
	CFLAGS="$CFLAGS -Wall -g3 -ggdb -O0 -DPHALCON_DEBUG=1"
	AC_DEFINE(PHALCON_DEBUG, 1, [Enable phalcon7 debug support])
else
	CFLAGS="$CFLAGS -DPHALCON_RELEASE=1"
fi

PHP_ARG_WITH(non-free, wheter to enable non-free css and js minifier,
[  --without-non-free      Disable non-free minifiers], yes, no)

AC_MSG_CHECKING([Include non-free minifiers])
if test "$PHP_NON_FREE" = "yes"; then
	AC_DEFINE([PHALCON_NON_FREE], [1], [Whether non-free minifiers are available])
	AC_MSG_RESULT([yes, css and js])
else
	AC_MSG_RESULT([no])
fi

PHP_ARG_ENABLE(chart, whether to enable chart support,
[  --enable-chart   Enable chart support], no, no)

AC_MSG_CHECKING([Include chart])
if test "$PHP_CHART" = "yes"; then
	AC_DEFINE([PHALCON_CHART], [1], [Whether chart are available])
	AC_MSG_RESULT([yes, chart])
else
	AC_MSG_RESULT([no])
fi

PHP_ARG_ENABLE(qrcode, wheter to enable qrcode support,
[  --enable-qrcode         Enable qrcode], no, no)

AC_MSG_CHECKING([Include qrcode])
if test "$PHP_QRCODE" = "yes"; then
	AC_DEFINE([PHALCON_QRCODE], [1], [Whether qrcode are available])
	AC_MSG_RESULT([yes, qrcode])
else
	AC_MSG_RESULT([no])
fi

PHP_ARG_ENABLE(intrusive, whether to enable intrusive support,
[  --enable-intrusive   Enable intrusive support], no, no)

AC_MSG_CHECKING([Include intrusive])
if test "$PHP_INTRUSIVE" = "yes"; then
	AC_DEFINE([PHALCON_INTRUSIVE], [1], [Whether intrusive are available])
	AC_MSG_RESULT([yes, intrusive])
else
	AC_MSG_RESULT([no])
fi

PHP_ARG_ENABLE(cache-yac, whether to enable yac support,
[  --enable-cache-yac   Enable yac support], no, no)

AC_MSG_CHECKING([Include cache-yac])
if test "$PHP_CACHE_YAC" = "yes"; then
	AC_DEFINE([PHALCON_CACHE_YAC], [1], [Whether yac are available])
	AC_MSG_RESULT([yes, yac])
else
	AC_MSG_RESULT([no])
fi

PHP_ARG_ENABLE(process, whether to enable process support,
[  --enable-process   Enable process support], yes, no)

AC_MSG_CHECKING([Include process])
if test "$PHP_PROCESS" = "yes"; then
	AC_DEFINE([PHALCON_PROCESS], [1], [Whether process are available])
	AC_MSG_RESULT([yes, process])
else
	AC_MSG_RESULT([no])
fi

PHP_ARG_ENABLE(websocket, whether to enable websocket support,
[  --enable-websocket   Enable websocket support], no, no)

AC_MSG_CHECKING([Include websocket])
if test "$PHP_WEBSOCKET" = "yes"; then
	AC_DEFINE([PHALCON_WEBSOCKET], [1], [Whether websocket are available])
	AC_MSG_RESULT([yes, websocket])
else
	AC_MSG_RESULT([no])
fi

PHP_ARG_ENABLE(storage-btree, whether to enable storage btree support,
[  --enable-storage-btree   Enable storage btree support], no, no)

if test "$PHP_STORAGE_BTREE" = "yes"; then
	AC_DEFINE([PHALCON_STORAGE_BTREE], [1], [Whether storage btree are available])
	AC_MSG_RESULT([yes, storage btree])
else
	AC_MSG_RESULT([no])
fi

PHP_ARG_ENABLE(storage-wiredtiger, whether to enable storage wiredtiger support,
[  --enable-storage-wiredtiger   Enable storage wiredtiger support], no, no)

if test "$PHP_STORAGE_WIREDTIGER" = "yes"; then
	AC_DEFINE([PHALCON_STORAGE_WIREDTIGER], [1], [Whether storage wiredtiger are available])
	AC_MSG_RESULT([yes, storage wiredtiger])
else
	AC_MSG_RESULT([no])
fi

PHP_ARG_ENABLE(storage-lmdb, whether to enable storage lmdb support,
[  --enable-storage-lmdb   Enable storage lmdb support], no, no)

if test "$PHP_STORAGE_LMDB" = "yes"; then
	AC_DEFINE([PHALCON_STORAGE_LMDB], [1], [Whether storage lmdb are available])
	AC_MSG_RESULT([yes, storage lmdb])
else
	AC_MSG_RESULT([no])
fi

PHP_ARG_ENABLE(storage-libmdbx, whether to enable storage libmdbx support,
[  --enable-storage-libmdbx   Enable storage libmdbx support], no, no)

if test "$PHP_STORAGE_LIBMDBX" = "yes"; then
	AC_DEFINE([PHALCON_STORAGE_LIBMDBX], [1], [Whether storage libmdbx are available])
	AC_MSG_RESULT([yes, storage libmdbx])
else
	AC_MSG_RESULT([no])
fi

PHP_ARG_ENABLE(storage-leveldb, whether to enable storage leveldb support,
[  --enable-storage-leveldb   Enable storage leveldb support], no, no)

if test "$PHP_STORAGE_LEVELDB" = "yes"; then
	AC_DEFINE([PHALCON_STORAGE_LEVELDB], [1], [Whether storage leveldb are available])
	AC_MSG_RESULT([yes, storage leveldb])
else
	AC_MSG_RESULT([no])
fi

PHP_ARG_ENABLE(storage-bloomfilter, whether to enable storage bloomfilter support,
[  --enable-storage-bloomfilter   Enable storage bloomfilter support], no, no)

if test "$PHP_STORAGE_BLOOMFILTER" = "yes"; then
	AC_DEFINE([PHALCON_STORAGE_BLOOMFILTER], [1], [Whether storage bloomfilter are available])
	AC_MSG_RESULT([yes, storage bloomfilter])
else
	AC_MSG_RESULT([no])
fi

PHP_ARG_ENABLE(storage-datrie, whether to enable storage datrie support,
[  --enable-storage-datrie   Enable storage datrie support], no, no)

if test "$PHP_STORAGE_DATRIE" = "yes"; then
	AC_DEFINE([PHALCON_STORAGE_DATRIE], [1], [Whether storage datrie are available])
	AC_MSG_RESULT([yes, storage datrie])
else
	AC_MSG_RESULT([no])
fi

PHP_ARG_ENABLE(server, whether to enable server support,
[  --enable-server   Enable server support], no, no)

if test "$PHP_SERVER" = "yes"; then
	AC_DEFINE([PHALCON_SERVER], [1], [Whether server are available])
	AC_MSG_RESULT([yes, server])
else
	AC_MSG_RESULT([no])
fi

PHP_ARG_ENABLE(uv, whether to enable libuv support,
[  --enable-uv   Enable libuv support], yes, no)

if test "$PHP_UV" = "yes"; then
	AC_MSG_RESULT([yes, libuv])
else
	AC_MSG_RESULT([no])
fi

PHP_ARG_ENABLE(python, for python support,
[  --enable-python   Include python support], no, no)

if test "$PHP_PYTHON" = "yes"; then
	AC_DEFINE([PHALCON_PYTHON], [1], [Whether python are available])
	AC_MSG_RESULT([yes, python])
else
	AC_MSG_RESULT([no])
fi

AC_DEFUN([AC_TIDEWAYS_CLOCK],
[
  have_clock_gettime=no

  AC_MSG_CHECKING([for clock_gettime])

  AC_TRY_LINK([ #include <time.h> ], [struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);], [
    have_clock_gettime=yes
    AC_MSG_RESULT([yes])
  ], [
    AC_MSG_RESULT([no])
  ])

  if test "$have_clock_gettime" = "no"; then
    AC_MSG_CHECKING([for clock_gettime in -lrt])

    SAVED_LIBS="$LIBS"
    LIBS="$LIBS -lrt"

    AC_TRY_LINK([ #include <time.h> ], [struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);], [
      have_clock_gettime=yes
      TIDEWAYS_SHARED_LIBADD="$TIDEWAYS_SHARED_LIBADD -lrt"
      AC_MSG_RESULT([yes])
    ], [
      LIBS="$SAVED_LIBS"
      AC_MSG_RESULT([no])
    ])
  fi

  if test "$have_clock_gettime" = "no"; then
    AC_MSG_CHECKING([for clock_get_time])

    AC_TRY_RUN([ #include <mach/mach.h>
      #include <mach/clock.h>
      #include <mach/mach_error.h>

      int main()
      {
        kern_return_t ret; clock_serv_t aClock; mach_timespec_t aTime;
        ret = host_get_clock_service(mach_host_self(), REALTIME_CLOCK, &aClock);

        if (ret != KERN_SUCCESS) {
          return 1;
        }

        ret = clock_get_time(aClock, &aTime);
        if (ret != KERN_SUCCESS) {
          return 2;
        }

        return 0;
      }
    ], [
      have_clock_gettime=yes
      AC_DEFINE([HAVE_CLOCK_GET_TIME], 1, [do we have clock_get_time?])
      AC_MSG_RESULT([yes])
    ], [
      AC_MSG_RESULT([no])
    ])
  fi

  if test "$have_clock_gettime" = "yes"; then
      AC_DEFINE([HAVE_CLOCK_GETTIME], 1, [do we have clock_gettime?])
  fi
])

AC_CHECK_FUNCS(gettimeofday)
AC_CHECK_FUNCS(clock_gettime)
AC_TIDEWAYS_CLOCK

AC_MSG_CHECKING(for sysvipc shared memory support)
AC_TRY_RUN([
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>

int main() {
  pid_t pid;
  int status;
  int ipc_id;
  char *shm;
  struct shmid_ds shmbuf;

  ipc_id = shmget(IPC_PRIVATE, 4096, (IPC_CREAT | SHM_R | SHM_W));
  if (ipc_id == -1) {
    return 1;
  }

  shm = shmat(ipc_id, NULL, 0);
  if (shm == (void *)-1) {
    shmctl(ipc_id, IPC_RMID, NULL);
    return 2;
  }

  if (shmctl(ipc_id, IPC_STAT, &shmbuf) != 0) {
    shmdt(shm);
    shmctl(ipc_id, IPC_RMID, NULL);
    return 3;
  }

  shmbuf.shm_perm.uid = getuid();
  shmbuf.shm_perm.gid = getgid();
  shmbuf.shm_perm.mode = 0600;

  if (shmctl(ipc_id, IPC_SET, &shmbuf) != 0) {
    shmdt(shm);
    shmctl(ipc_id, IPC_RMID, NULL);
    return 4;
  }

  shmctl(ipc_id, IPC_RMID, NULL);

  strcpy(shm, "hello");

  pid = fork();
  if (pid < 0) {
    return 5;
  } else if (pid == 0) {
    strcpy(shm, "bye");
    return 6;
  }
  if (wait(&status) != pid) {
    return 7;
  }
  if (!WIFEXITED(status) || WEXITSTATUS(status) != 6) {
    return 8;
  }
  if (strcmp(shm, "bye") != 0) {
    return 9;
  }
  return 0;
}
],dnl
AC_DEFINE(HAVE_SHM_IPC, 1, [Define if you have SysV IPC SHM support])
    msg=yes,msg=no,msg=no)
AC_MSG_RESULT([$msg])

AC_MSG_CHECKING(for mmap() using MAP_ANON shared memory support)
AC_TRY_RUN([
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#ifndef MAP_ANON
# ifdef MAP_ANONYMOUS
#  define MAP_ANON MAP_ANONYMOUS
# endif
#endif
#ifndef MAP_FAILED
# define MAP_FAILED ((void*)-1)
#endif

int main() {
  pid_t pid;
  int status;
  char *shm;

  shm = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  if (shm == MAP_FAILED) {
    return 1;
  }

  strcpy(shm, "hello");

  pid = fork();
  if (pid < 0) {
    return 5;
  } else if (pid == 0) {
    strcpy(shm, "bye");
    return 6;
  }
  if (wait(&status) != pid) {
    return 7;
  }
  if (!WIFEXITED(status) || WEXITSTATUS(status) != 6) {
    return 8;
  }
  if (strcmp(shm, "bye") != 0) {
    return 9;
  }
  return 0;
}
],dnl
AC_DEFINE(HAVE_SHM_MMAP_ANON, 1, [Define if you have mmap(MAP_ANON) SHM support])
    msg=yes,msg=no,msg=no)
AC_MSG_RESULT([$msg])

AC_MSG_CHECKING(for epoll support)
AC_TRY_RUN([
#include <stdlib.h>
#include <sys/epoll.h>

int
main(int argc, char **argv)
{
	int epfd;

	epfd = epoll_create(1);
	exit (epfd == -1 ? 1 : 0);
}
],dnl
AC_DEFINE(HAVE_EPOLL, 1, [Define if your have epoll support])
    msg=yes,msg=no,msg=no)
AC_MSG_RESULT([$msg])

AC_MSG_CHECKING([for kqueue])
AC_TRY_COMPILE(
[
    #include <sys/types.h>
    #include <sys/event.h>
    #include <sys/time.h>
], [
    int kfd;
    struct kevent k;
    kfd = kqueue();
    /* 0 -> STDIN_FILENO */
    EV_SET(&k, 0, EVFILT_READ , EV_ADD | EV_CLEAR, 0, 0, NULL);
], [
    AC_DEFINE([HAVE_KQUEUE], 1, [do we have kqueue?])
    AC_MSG_RESULT([yes])
], [
    AC_MSG_RESULT([no])
])

if test "$PHP_PHALCON" = "yes"; then
	AC_MSG_CHECKING([PHP version])

	tmp_version=$PHP_VERSION
	if test -z "$tmp_version"; then
		if test -z "$PHP_CONFIG"; then
		AC_MSG_ERROR([php-config not found])
		fi
		php_version=`$PHP_CONFIG --version 2>/dev/null|head -n 1|sed -e 's#\([0-9]\.[0-9]*\.[0-9]*\)\(.*\)#\1#'`
	else
		php_version=`echo "$tmp_version"|sed -e 's#\([0-9]\.[0-9]*\.[0-9]*\)\(.*\)#\1#'`
	fi

	if test -z "$php_version"; then
		AC_MSG_ERROR([failed to detect PHP version, please report])
	fi

	ac_IFS=$IFS
	IFS="."
	set $php_version
	IFS=$ac_IFS
	phalcon_php_version=`expr [$]1 \* 1000000 + [$]2 \* 1000 + [$]3`

	if test "$phalcon_php_version" -lt "7000000"; then
		AC_MSG_ERROR([You need at least PHP 7.0 to be able to use this version of Phalcon/Dao7. PHP $php_version found])
	else
		AC_MSG_RESULT([$php_version, ok.])
	fi

	AC_DEFINE(HAVE_PHALCON, 1, [Whether you have Phalcon Framework])
	phalcon_sources="phalcon.c \
kernel/main.c \
kernel/fcall.c \
kernel/require.c \
kernel/debug.c \
kernel/backtrace.c \
kernel/object.c \
kernel/array.c \
kernel/hash.c \
kernel/murmurhash.c \
kernel/string.c \
kernel/mbstring.c \
kernel/filter.c \
kernel/operators.c \
kernel/concat.c \
kernel/exception.c \
kernel/file.c \
kernel/output.c \
kernel/memory.c \
kernel/shm.c \
kernel/mpool.c \
kernel/avltree.c \
kernel/rbtree.c \
kernel/bloomfilter.c \
kernel/countingbloomfilter.c \
kernel/datrie/trie.c \
kernel/datrie/alpha-map.c \
kernel/datrie/darray.c \
kernel/datrie/dstring.c \
kernel/datrie/fileutils.c \
kernel/datrie/tail.c \
kernel/datrie/trie-string.c \
kernel/list.c \
kernel/session.c \
kernel/variables.c \
kernel/framework/orm.c \
kernel/framework/router.c \
kernel/framework/url.c \
kernel/assert.c \
kernel/exit.c \
kernel/iterator.c \
kernel/math.c \
kernel/time.c \
kernel/message/queue.c \
kernel/io/support.c \
kernel/io/epoll.c \
kernel/io/kqueue.c \
kernel/io/generic.c \
kernel/io/sockets.c \
kernel/io/networks.c \
kernel/io/client.c \
kernel/io/server.c \
kernel/io/tasks.c \
kernel/io/threads.c \
kernel/gc.c \
kernel/thread/pool.c \
interned-strings.c \
xhprof.c \
logger.c \
flash.c \
security/exception.c \
profiler.c \
profilerinterface.c \
profiler/item.c \
profiler/iteminterface.c \
profiler/exception.c \
db/dialect/sqlite.c \
db/dialect/mysql.c \
db/dialect/postgresql.c \
db/result/pdo.c \
db/column.c \
db/index.c \
db/profiler/item.c \
db/indexinterface.c \
db/dialectinterface.c \
db/resultinterface.c \
db/profiler.c \
db/referenceinterface.c \
db/adapter/pdo/sqlite.c \
db/adapter/pdo/mysql.c \
db/adapter/pdo/postgresql.c \
db/adapter/pdo.c \
db/exception.c \
db/reference.c \
db/adapterinterface.c \
db/dialect.c \
db/adapter.c \
db/rawvalue.c \
db/columninterface.c \
db/builderinterface.c \
db/builder.c \
db/builder/exception.c \
db/builder/where.c \
db/builder/join.c \
db/builder/select.c \
db/builder/update.c \
db/builder/insert.c \
db/builder/delete.c \
forms/form.c \
forms/manager.c \
forms/element/file.c \
forms/element/email.c \
forms/element/hidden.c \
forms/element/password.c \
forms/element/text.c \
forms/element/select.c \
forms/element/textarea.c \
forms/element/check.c \
forms/element/radio.c \
forms/element/numeric.c \
forms/element/submit.c \
forms/element/date.c \
forms/exception.c \
forms/element.c \
forms/elementinterface.c \
http/parser/http_parser.c \
http/parser.c \
http/response.c \
http/requestinterface.c \
http/request.c \
http/cookie.c \
http/request/file.c \
http/request/exception.c \
http/request/fileinterface.c \
http/responseinterface.c \
http/cookie/exception.c \
http/response/cookies.c \
http/response/exception.c \
http/response/headers.c \
http/response/cookiesinterface.c \
http/response/headersinterface.c \
http/uri.c \
http/client.c \
http/client/exception.c \
http/client/header.c \
http/client/response.c \
http/client/adapterinterface.c \
http/client/adapter.c \
http/client/adapter/curl.c \
http/client/adapter/stream.c \
dispatcherinterface.c \
di.c \
loader/exception.c \
cryptinterface.c \
db.c \
text.c \
arr.c \
matrix.c \
date.c \
date/datetime.c \
binary.c \
binary/reader.c \
binary/writer.c \
binary/exception.c \
debug.c \
debug/exception.c \
debug/dump.c \
tag.c \
user/component.c \
user/plugin.c \
user/module.c \
user/logic.c \
application.c \
application/exception.c \
router.c \
routerinterface.c \
router/exception.c \
cli/dispatcher/exception.c \
cli/console.c \
cli/router.c \
cli/task.c \
cli/router/exception.c \
cli/dispatcher.c \
cli/console/exception.c \
cli/options.c \
cli/options/exception.c \
cli/color.c \
mvc/controller.c \
mvc/dispatcher/exception.c \
mvc/application/exception.c \
mvc/router.c \
mvc/micro.c \
mvc/micro/middlewareinterface.c \
mvc/micro/lazyloader.c \
mvc/micro/exception.c \
mvc/micro/collection.c \
mvc/micro/collectioninterface.c \
mvc/dispatcherinterface.c \
mvc/routerinterface.c \
mvc/urlinterface.c \
mvc/user/component.c \
mvc/user/plugin.c \
mvc/user/module.c \
mvc/user/logic.c \
mvc/user/logic/model.c \
mvc/url.c \
mvc/model.c \
mvc/view.c \
mvc/modelinterface.c \
mvc/router/group.c \
mvc/router/route.c \
mvc/router/annotations.c \
mvc/router/exception.c \
mvc/router/routeinterface.c \
mvc/url/exception.c \
mvc/viewinterface.c \
mvc/dispatcher.c \
mvc/view/engine/php.c \
mvc/view/exception.c \
mvc/view/engineinterface.c \
mvc/view/simple.c \
mvc/view/engine.c \
mvc/view/model.c \
mvc/view/modelinterface.c \
mvc/application.c \
mvc/controllerinterface.c \
mvc/moduledefinitioninterface.c \
mvc/model/metadata/files.c \
mvc/model/metadata/strategy/introspection.c \
mvc/model/metadata/strategy/annotations.c \
mvc/model/metadata/apc.c \
mvc/model/metadata/memory.c \
mvc/model/metadata/session.c \
mvc/model/metadata/memcached.c \
mvc/model/metadata/redis.c \
mvc/model/metadata/cache.c \
mvc/model/transaction.c \
mvc/model/metadata.c \
mvc/model/resultsetinterface.c \
mvc/model/managerinterface.c \
mvc/model/behavior.c \
mvc/model/query/exception.c \
mvc/model/query/builder.c \
mvc/model/query/builder/where.c \
mvc/model/query/builder/join.c \
mvc/model/query/builder/select.c \
mvc/model/query/builder/update.c \
mvc/model/query/builder/insert.c \
mvc/model/query/builder/delete.c \
mvc/model/query/lang.c \
mvc/model/query/statusinterface.c \
mvc/model/query/status.c \
mvc/model/query/builderinterface.c \
mvc/model/resultinterface.c \
mvc/model/criteriainterface.c \
mvc/model/query.c \
mvc/model/resultset.c \
mvc/model/validationfailed.c \
mvc/model/manager.c \
mvc/model/behaviorinterface.c \
mvc/model/relation.c \
mvc/model/exception.c \
mvc/model/transaction/failed.c \
mvc/model/transaction/managerinterface.c \
mvc/model/transaction/manager.c \
mvc/model/transaction/exception.c \
mvc/model/queryinterface.c \
mvc/model/row.c \
mvc/model/criteria.c \
mvc/model/resultset/complex.c \
mvc/model/resultset/simple.c \
mvc/model/behavior/timestampable.c \
mvc/model/behavior/softdelete.c \
mvc/model/metadatainterface.c \
mvc/model/relationinterface.c \
mvc/model/transactioninterface.c \
config.c \
config/adapter.c \
config/adapterinterface.c \
config/exception.c \
config/adapter/ini.c \
config/adapter/json.c \
config/adapter/php.c \
config/adapter/yaml.c \
filterinterface.c \
logger/multiple.c \
logger/formatter/firephp.c \
logger/formatter/json.c \
logger/formatter/line.c \
logger/formatter/syslog.c \
logger/formatter.c \
logger/adapter/file.c \
logger/adapter/firephp.c \
logger/adapter/stream.c \
logger/adapter/syslog.c \
logger/adapter/direct.c \
logger/exception.c \
logger/adapterinterface.c \
logger/formatterinterface.c \
logger/adapter.c \
logger/item.c \
filter/exception.c \
filter/userfilterinterface.c \
queue/beanstalk.c \
queue/beanstalk/job.c \
assets/resource/css.c \
assets/resource/js.c \
assets/filters/none.c \
assets/filters/cssmin.c \
assets/filters/jsmin.c \
assets/filterinterface.c \
assets/resource.c \
assets/manager.c \
assets/exception.c \
assets/collection.c \
escaper/exception.c \
loader.c \
tag/select.c \
tag/exception.c \
acl.c \
acl/resource.c \
acl/resourceaware.c \
acl/resourceinterface.c \
acl/adapter/memory.c \
acl/exception.c \
acl/role.c \
acl/roleaware.c \
acl/adapterinterface.c \
acl/adapter.c \
acl/roleinterface.c \
exception.c \
continueexception.c \
exitexception.c \
crypt.c \
filter.c \
dispatcher.c \
cache/multiple.c \
cache/frontend/none.c \
cache/frontend/base64.c \
cache/frontend/json.c \
cache/frontend/igbinary.c \
cache/frontend/data.c \
cache/frontend/output.c \
cache/backend/file.c \
cache/backend/apc.c \
cache/backend/memcached.c \
cache/backend/memory.c \
cache/backend/redis.c \
cache/backend/yac.c \
cache/exception.c \
cache/backendinterface.c \
cache/frontendinterface.c \
cache/backend.c \
session/bag.c \
session/adapter/files.c \
session/exception.c \
session/baginterface.c \
session/adapterinterface.c \
session/adapter.c \
session/adapter/memcached.c \
session/adapter/cache.c \
diinterface.c \
escaper.c \
crypt/exception.c \
events/managerinterface.c \
events/manager.c \
events/eventinterface.c \
events/event.c \
events/exception.c \
events/eventsawareinterface.c \
events/listener.c \
escaperinterface.c \
validationinterface.c \
validation.c \
version.c \
flashinterface.c \
kernel.c \
paginator/adapter.c \
paginator/adapter/model.c \
paginator/adapter/nativearray.c \
paginator/adapter/querybuilder.c \
paginator/adapter/dbbuilder.c \
paginator/adapter/sql.c \
paginator/exception.c \
paginator/adapterinterface.c \
di/injectable.c \
di/factorydefault.c \
di/service/builder.c \
di/serviceinterface.c \
di/factorydefault/cli.c \
di/exception.c \
di/injectionawareinterface.c \
di/service.c \
random.c \
security.c \
security/random.c \
annotations/reflection.c \
annotations/annotation.c \
annotations/readerinterface.c \
annotations/adapter/files.c \
annotations/adapter/apc.c \
annotations/adapter/memory.c \
annotations/adapter/cache.c \
annotations/exception.c \
annotations/collection.c \
annotations/adapterinterface.c \
annotations/adapter.c \
annotations/reader.c \
flash/direct.c \
flash/exception.c \
flash/session.c \
translate/adapter/nativearray.c \
translate/exception.c \
translate/adapterinterface.c \
translate/adapter.c \
translate/adapter/gettext.c \
translate/adapter/php.c \
validation/validatorinterface.c \
validation/message/group.c \
validation/exception.c \
validation/message.c \
validation/messageinterface.c \
validation/validator/email.c \
validation/validator/presenceof.c \
validation/validator/confirmation.c \
validation/validator/regex.c \
validation/validator/exclusionin.c \
validation/validator/identical.c \
validation/validator/between.c \
validation/validator/inclusionin.c \
validation/validator/stringlength.c \
validation/validator/url.c \
validation/validator/file.c \
validation/validator/numericality.c \
validation/validator/json.c \
validation/validator/uniqueness.c \
validation/validator/alnum.c \
validation/validator/alpha.c \
validation/validator/digit.c \
validation/validator/date.c \
validation/validator.c \
mvc/model/query/parser.c \
mvc/model/query/scanner.c \
annotations/parser.c \
annotations/scanner.c \
image.c \
image/adapter.c \
image/adapterinterface.c \
image/exception.c \
image/adapter/gd.c \
image/adapter/imagick.c \
registry.c \
async.c \
async/core.c \
async/channel.c \
async/console.c \
async/context.c \
async/deferred.c \
async/dns.c \
async/fiber/stack.c \
async/filesystem.c \
async/pipe.c \
async/process/builder.c \
async/process/env.c \
async/process/runner.c \
async/socket.c \
async/ssl/api.c \
async/ssl/bio.c \
async/ssl/engine.c \
async/stream.c \
async/sync.c \
async/task.c \
async/tcp.c \
async/udp.c \
async/watcher/monitor.c \
async/watcher/poll.c \
async/watcher/signal.c \
async/watcher/timer.c \
async/xp/socket.c \
async/xp/tcp.c \
async/xp/udp.c \
thread/exception.c \
thread/pool.c \
chart/exception.c \
chart/captcha/tiny.c \
socket/exception.c \
socket.c \
socket/client.c \
socket/server.c \
process/exception.c \
storage/exception.c \
snowflake.c \
server/utils.c \
server/simple.c \
server/exception.c \
aop/lexer.c \
aop/exception.c \
aop/joinpoint.c \
aop.c"

	if test "$PHP_CACHE_YAC" = "yes"; then
		phalcon_sources="$phalcon_sources cache/yac/allocators/mmap.c cache/yac/allocators/shm.c cache/yac/serializer.c cache/yac/storage.c cache/yac/allocator.c cache/yac.c"
	fi

	if test "$PHP_CHART" = "yes"; then
		phalcon_sources="$phalcon_sources chart/captcha.c"
	fi

	if test "$PHP_INTRUSIVE" = "yes"; then
		phalcon_sources="$phalcon_sources intrusive/avltree.c intrusive/avltree/node.c intrusive/rbtree.c intrusive/rbtree/node.c"
	fi

	if test "$PHP_PROCESS" = "yes"; then
		phalcon_sources="$phalcon_sources process/system.c process/sharedmemory.c process/proc.c"
	fi

	if test "$PHP_STORAGE_BTREE" = "yes"; then
		phalcon_sources="$phalcon_sources storage/btree/bplus.c storage/btree/pages.c storage/btree/utils.c storage/btree/values.c storage/btree/writer.c storage/btree.c"
	fi

	old_CPPFLAGS=$CPPFLAGS
	CPPFLAGS="$CPPFLAGS $INCLUDES"

	AC_CHECK_HEADERS(
		[ext/igbinary/igbinary.h],
		[
			PHP_ADD_EXTENSION_DEP([phalcon], [igbinary])
			AC_DEFINE([PHALCON_USE_PHP_IGBINARY], [1], [Whether PHP igbinary extension is present at compile time])
		],
		,
		[[#include "main/php.h"]]
	)


	AC_CHECK_HEADERS(
		[ext/pcre/php_pcre.h],
		[
			PHP_ADD_EXTENSION_DEP([phalcon], [pcre])
			AC_DEFINE([PHALCON_USE_PHP_PCRE], [1], [Whether PHP pcre extension is present at compile time])
		],
		[
			AC_MSG_ERROR([Incorrect pcre extension])
		],
		[[#include "main/php.h"]]
	)

	AC_CHECK_HEADERS(
		[ext/json/php_json.h],
		[
			PHP_ADD_EXTENSION_DEP([phalcon], [json])
			AC_DEFINE([PHALCON_USE_PHP_JSON], [1], [Whether PHP json extension is present at compile time])
		],
		,
		[[#include "main/php.h"]]
	)

	AC_MSG_CHECKING([checking socket type support])
	AC_TRY_COMPILE(
	[
		#include <sys/types.h>
		#include <sys/socket.h>
	],[
		static struct msghdr tp; int n = (int) tp.msg_flags; return n
	],[
		AC_DEFINE([HAVE_SOCKETS], 1, [ ])
		AC_MSG_RESULT([yes])
	],[
		AC_DEFINE(MISSING_MSGHDR_MSGFLAGS, 1, [ ])
		AC_MSG_RESULT([no])
	])

	AC_CHECK_DECL(
		[HAVE_PHP_SESSION],
		[
			AC_CHECK_HEADERS(
				[ext/session/php_session.h],
				[
					PHP_ADD_EXTENSION_DEP([phalcon], [session])
					AC_DEFINE([PHALCON_USE_PHP_SESSION], [1], [Whether PHP session extension is present at compile time])
				],
				,
				[[#include "main/php.h"]]
			)
		],
		,
		[[#include "php_config.h"]]
	)

	PHP_SOCKETS='no'

	AC_CHECK_HEADERS(
		[ext/sockets/php_sockets.h],
		[
			PHP_SOCKETS='yes'
			PHP_ADD_EXTENSION_DEP([phalcon], [sockets])
			AC_DEFINE([PHALCON_USE_PHP_SOCKETS], [1], [Whether PHP sockets extension is present at compile time])
		],
		,
		[[#include "main/php.h"]]
	)

	AC_CHECK_DECL(
		[HAVE_HASH_EXT],
		[
			AC_CHECK_HEADERS(
				[ext/hash/php_hash.h],
				[
					PHP_ADD_EXTENSION_DEP([phalcon], [hash])
					AC_DEFINE([PHALCON_USE_PHP_HASH], [1], [Whether PHP hash extension is present at compile time])
				],
				,
				[[#include "main/php.h"]]
			)
		],
		,
		[[#include "php_config.h"]]
	)

	AC_CHECK_DECL(
		[HAVE_HASH_MBSTRING],
		[
			AC_CHECK_HEADERS(
				[ext/mbstring/mbstring.h],
				[
					PHP_ADD_EXTENSION_DEP([phalcon], [mbstring])
					AC_DEFINE([PHALCON_USE_PHP_MBSTRING], [1], [Whether PHP mbstring extension is present at compile time])
				],
				,
				[[#include "main/php.h"]]
			)
		],
		,
		[[#include "php_config.h"]]
	)

	CPPFLAGS=$old_CPPFLAGS

	AC_MSG_CHECKING([checking png support])
	for i in /usr/local /usr; do
		if test -r $i/include/png.h; then
			PNG_CFLAGS=`pkg-config --cflags libpng`
			PNG_LDFLAGS=`pkg-config --libs libpng`

			PHP_ADD_INCLUDE($i/include)

			CPPFLAGS="${CPPFLAGS} ${PNG_CFLAGS}"
			EXTRA_LDFLAGS="${EXTRA_LDFLAGS} ${PNG_LDFLAGS}"

			AC_MSG_RESULT(yes)

			AC_DEFINE([PHALCON_USE_PNG], [1], [Have libpng support])
			break
		fi
	done

	if test "$PHP_QRCODE" = "yes"; then
		if test -z "$PNG_CFLAGS"; then
			AC_MSG_ERROR([Incorrect png library])
		fi
	fi

	AC_MSG_CHECKING([checking ImageMagick MagickWand support])
	for i in /usr/local /usr; do
		if test -r $i/include/ImageMagick/wand/MagickWand.h; then
			WAND_CFLAGS=`pkg-config --cflags MagickWand`
			WAND_LDFLAGS=`pkg-config --libs MagickWand`

			PHP_ADD_INCLUDE($i/include/ImageMagick)

			CPPFLAGS="${CPPFLAGS} ${WAND_CFLAGS}"
			EXTRA_LDFLAGS="${EXTRA_LDFLAGS} ${WAND_LDFLAGS}"

			AC_MSG_RESULT(yes, found in $i)

			AC_DEFINE([PHALCON_USE_MAGICKWAND], [1], [Have ImageMagick MagickWand support])
			break
		elif test -r $i/include/ImageMagick-6/wand/MagickWand.h; then
			WAND_CFLAGS=`pkg-config --cflags MagickWand`
			WAND_LDFLAGS=`pkg-config --libs MagickWand`

			PHP_ADD_INCLUDE($i/include/ImageMagick-6)

			CPPFLAGS="${CPPFLAGS} ${WAND_CFLAGS}"
			EXTRA_LDFLAGS="${EXTRA_LDFLAGS} ${WAND_LDFLAGS}"

			AC_MSG_RESULT(yes, found in $i)

			AC_DEFINE([PHALCON_USE_MAGICKWAND], [1], [Have ImageMagick MagickWand support])
			break
		elif test -r $i/bin/MagickWand-config; then
			IM_WAND_BINARY=$i/bin/MagickWand-config

			IM_IMAGEMAGICK_VERSION=`$IM_WAND_BINARY --version`
			IM_IMAGEMAGICK_VERSION_MASK=`echo $IM_IMAGEMAGICK_VERSION | $AWK 'BEGIN { FS = "."; } { printf "%d", ($[1] * 1000 + $[2]) * 1000 + $[3];}'`

			AC_MSG_CHECKING(if ImageMagick version is at least 6.2.4)
			if test "$IM_IMAGEMAGICK_VERSION_MASK" -ge $1; then
				AC_MSG_RESULT(found version $IM_IMAGEMAGICK_VERSION)
			else
				AC_MSG_ERROR(no. You need at least Imagemagick version 6.2.4 to use Imagick.)
			fi

			WAND_CFLAGS=`$IM_WAND_BINARY --cflags`
			WAND_LDFLAGS=`$IM_WAND_BINARY --libs`

			CPPFLAGS="${CPPFLAGS} ${WAND_CFLAGS}"
			EXTRA_LDFLAGS="${EXTRA_LDFLAGS} ${WAND_LDFLAGS}"

			AC_MSG_CHECKING(for MagickWand.h header)

			IM_PREFIX=`$IM_WAND_BINARY --prefix`
			IM_MAJOR_VERSION=`echo $IM_IMAGEMAGICK_VERSION | $AWK 'BEGIN { FS = "."; } {print $[1]}'`

			if test -r "${IM_PREFIX}/include/ImageMagick-${IM_MAJOR_VERSION}/MagickWand/MagickWand.h"; then
				PHP_ADD_INCLUDE(${IM_PREFIX}/include/ImageMagick-${IM_MAJOR_VERSION})

				AC_DEFINE([IM_MAGICKWAND_HEADER_STYLE_SEVEN], [1], [ImageMagick 7.x style header])
				AC_DEFINE([PHALCON_USE_MAGICKWAND], [1], [Have ImageMagick MagickWand support])

				AC_MSG_RESULT([${IM_PREFIX}/include/ImageMagick-${IM_MAJOR_VERSION}/MagickWand/MagickWand.h])
				break
			elif test -r "${IM_PREFIX}/include/ImageMagick-${IM_MAJOR_VERSION}/wand/MagickWand.h"; then
				PHP_ADD_INCLUDE(${IM_PREFIX}/include/ImageMagick-${IM_MAJOR_VERSION})

				AC_DEFINE([IM_MAGICKWAND_HEADER_STYLE_SIX], [1], [ImageMagick 6.x style header])
				AC_DEFINE([PHALCON_USE_MAGICKWAND], [1], [Have ImageMagick MagickWand support])

				AC_MSG_RESULT([${IM_PREFIX}/include/ImageMagick-${IM_MAJOR_VERSION}/wand/MagickWand.h])
				break
			elif test -r "${IM_PREFIX}/include/ImageMagick/wand/MagickWand.h"; then
				PHP_ADD_INCLUDE(${IM_PREFIX}/include/ImageMagick)

				AC_DEFINE([IM_MAGICKWAND_HEADER_STYLE_SIX], [1], [ImageMagick 6.x style header])
				AC_DEFINE([PHALCON_USE_MAGICKWAND], [1], [Have ImageMagick MagickWand support])

				AC_MSG_RESULT([${IM_PREFIX}/include/ImageMagick/wand/MagickWand.h])
				break
			else
				AC_MSG_ERROR([Unable to find MagickWand.h header])
			fi
		else
			AC_MSG_RESULT([no, found in $i])
		fi
	done

	AC_MSG_CHECKING([checking libmongoc support])
	for i in /usr/local /usr; do
		if test -r $i/include/libmongoc-1.0/mongoc.h; then
			MONGOC_CFLAGS=`pkg-config --cflags libmongoc-1.0`
			MONGOC_LDFLAGS=`pkg-config --libs libmongoc-1.0`

			PHP_ADD_INCLUDE($i/include/libmongoc-1.0)

			CPPFLAGS="${CPPFLAGS} ${MONGOC_CFLAGS}"
			EXTRA_LDFLAGS="${EXTRA_LDFLAGS} ${MONGOC_LDFLAGS}"

			AC_MSG_RESULT(yes, found in $i)

			AC_DEFINE([PHALCON_USE_MONGOC], [1], [Have libmongoc support])
			phalcon_sources="$phalcon_sources cache/backend/mongo.c mvc/model/metadata/mongo.c"

            PHP_CHECK_LIBRARY(mongoc-1.0, mongoc_collection_find_with_opts,
            [
                AC_DEFINE(PHALCON_MONGOC_HAS_FIND_OPTS, 1, [Has mongoc_collection_find_with_opts support])
            ])
			break
		else
			AC_MSG_RESULT([no, found in $i])
		fi
	done

	if test "$PHP_QRCODE" = "yes"; then
		AC_MSG_CHECKING([checking libqrencode support])
		for i in /usr/local /usr; do
			if test -r $i/include/qrencode.h; then
				PHP_ADD_INCLUDE($i/include)
				PHP_CHECK_LIBRARY(qrencode, QRcode_encodeString,
				[
					PHP_ADD_LIBRARY_WITH_PATH(qrencode, $i/$PHP_LIBDIR, PHALCON_SHARED_LIBADD)
					AC_DEFINE(PHALCON_USE_QRENCODE, 1, [Have qrencode support])
					phalcon_sources="$phalcon_sources chart/qrcode.c "
				],[
					AC_MSG_ERROR([Wrong qrencode version or library not found])
				],[
					-L$i/$PHP_LIBDIR -lm
				])
				break
			else
				AC_MSG_RESULT([no, found in $i])
			fi
		done

		if test -n "$WAND_CFLAGS"; then
			AC_MSG_CHECKING([checking libzbar support])
			for i in /usr/local /usr; do
				if test -r $i/include/zbar.h; then
					PHP_ADD_INCLUDE($i/include)
					PHP_CHECK_LIBRARY(zbar, zbar_scan_image,
					[
						PHP_ADD_LIBRARY_WITH_PATH(zbar, $i/$PHP_LIBDIR, PHALCON_SHARED_LIBADD)
						AC_DEFINE(PHALCON_USE_ZBAR, 1, [Have libzbar support])
					],[
						AC_MSG_ERROR([Wrong zbar version or library not found])
					],[
						-L$i/$PHP_LIBDIR -lm
					])
					break
				else
					AC_MSG_RESULT([no, found in $i])
				fi
			done
		fi
	fi

	AC_MSG_CHECKING([checking SSL support])
	for i in /usr/local /usr; do
		if test -r $i/include/openssl/evp.h; then
			PHP_ADD_INCLUDE($i/include)
			PHP_CHECK_LIBRARY(ssl, SSL_is_init_finished,
			[
				PHP_ADD_LIBRARY_WITH_PATH(ssl, $i/$PHP_LIBDIR, PHALCON_SHARED_LIBADD)
				AC_DEFINE(HAVE_ASYNC_SSL, 1, [Have SSL support])
			],[
				AC_MSG_RESULT([SSL library not found])
			],[
				-L$i/$PHP_LIBDIR
			])
			break
		else
			AC_MSG_RESULT([no, found in $i])
		fi
	done

  
	async_use_asm="yes"
	async_use_ucontext="no"

	AC_CHECK_HEADER(ucontext.h, [
		async_use_ucontext="yes"
	])
  
	AS_CASE([$host_cpu],
		[x86_64*], [async_cpu="x86_64"],
		[x86*], [async_cpu="x86"],
		[arm*], [async_cpu="arm"],
		[arm64*], [async_cpu="arm64"],
		[async_cpu="unknown"]
	)
	
	AS_CASE([$host_os],
		[darwin*], [async_os="MAC"],
		[cygwin*], [async_os="WIN"],
		[mingw*], [async_os="WIN"],
		[async_os="LINUX"]
	)

	if test "$async_cpu" = 'x86_64'; then
		if test "$async_os" = 'LINUX'; then
			async_asm_file="x86_64_sysv_elf_gas.S"
		elif test "$async_os" = 'MAC'; then
			async_asm_file="x86_64_sysv_macho_gas.S"
		else
			async_use_asm="no"
		fi
	elif test "$async_cpu" = 'x86'; then
		if test "$async_os" = 'LINUX'; then
			async_asm_file="i386_sysv_elf_gas.S"
		elif test "$async_os" = 'MAC'; then
			async_asm_file="i386_sysv_macho_gas.S"
		else
			async_use_asm="no"
		fi
	elif test "$async_cpu" = 'arm'; then
		if test "$async_os" = 'LINUX'; then
			async_asm_file="arm_aapcs_elf_gas.S"
		elif test "$async_os" = 'MAC'; then
			async_asm_file="arm_aapcs_macho_gas.S"
		else
			async_use_asm="no"
		fi
	else
		async_use_asm="no"
	fi

	AC_MSG_CHECKING(checking php version >= 7.1)
	if test "$PHP_UV" = "yes" && test "$phalcon_php_version" -ge "7001000"; then
		AC_MSG_RESULT(yes)
		if test "$async_use_asm" = 'yes'; then
			async_source_files="async/fiber/asm.c async/thirdparty/boost/asm/make_${async_asm_file} async/thirdparty/boost/asm/jump_${async_asm_file}"
		elif test "$async_use_ucontext" = 'yes'; then
			async_source_files="async/fiber/ucontext.c"
		else
			async_source_files=""
		fi
	else
		async_use_ucontext="no"
	fi

	AC_MSG_CHECKING([checking libuv support])
	if test "$async_use_ucontext" = "yes"; then
		DIR="${srcdir}/async/thirdparty"
		if test "$async_os" = 'LINUX' && test ! -s "${DIR}/lib/libuv.a"; then
			AC_MSG_RESULT(no)
			TMP=$(mktemp -d)
			cp -r ${DIR}/libuv $TMP
			pushd ${TMP}/libuv
			./autogen.sh
			./configure --disable-shared --prefix=${TMP}/build CFLAGS="$(CFLAGS) -fPIC -DPIC -g -O2"
			make install
			popd
			mv ${TMP}/build/lib/libuv.a ${DIR}/lib
			rm -rf $TMP
		else
			AC_MSG_RESULT(yes)
		fi

		if test "$async_os" = 'LINUX' && test -s "${DIR}/lib/libuv.a"; then
			AC_MSG_CHECKING([checking static libuv])
			PHP_ADD_INCLUDE("${DIR}/libuv/include")
			PHP_ADD_LIBRARY_WITH_PATH(uv, ${DIR}/lib, PHALCON_SHARED_LIBADD)
			AC_DEFINE(PHALCON_USE_UV, 1, [Have uv support])
			phalcon_sources="$phalcon_sources $async_source_files"
			AC_MSG_RESULT([yes, static])
			CPPFLAGS="${CPPFLAGS} -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1"
		else
			AC_MSG_CHECKING([checking libuv support])
			for i in /usr/local /usr; do
				if test -r $i/include/uv.h; then
					PHP_ADD_INCLUDE($i/include)
					PHP_CHECK_LIBRARY(uv, uv_fs_open,
					[
						PHP_ADD_LIBRARY_WITH_PATH(uv, $i/$PHP_LIBDIR, PHALCON_SHARED_LIBADD)
						AC_DEFINE(PHALCON_USE_UV, 1, [Have uv support])
						phalcon_sources="$phalcon_sources $async_source_files"
					],[
						AC_MSG_RESULT([Wrong uv version or library not found])
					],[
						-L$i/$PHP_LIBDIR -lpthread
					])
					break
				else
					AC_MSG_RESULT([no, found in $i])
				fi
			done
		fi
	fi

	if test "$PHP_WEBSOCKET" = "yes"; then
		AC_MSG_CHECKING([checking libwebsockets support])
		for i in /usr/local /usr; do
			if test -r $i/include/libwebsockets.h; then
				WEBSOCKET_DIR=$i
				PHP_ADD_INCLUDE($i/include)
				PHP_CHECK_LIBRARY(websockets, lws_callback_on_writable,
				[
					PHP_ADD_LIBRARY_WITH_PATH(websockets, $i/$PHP_LIBDIR, PHALCON_SHARED_LIBADD)
					AC_DEFINE(PHALCON_USE_WEBSOCKET, 1, [Have WebSocket support])
					phalcon_sources="$phalcon_sources websocket/connection.c websocket/server.c websocket/client.c websocket/eventloopinterface.c "
				],[
					AC_MSG_ERROR([Wrong websockets version or library not found])
				],[
					-L$i/$PHP_LIBDIR -lm
				])
				break
			else
				AC_MSG_RESULT([no, found in $i])
			fi
		done

		if test -z "$WEBSOCKET_DIR"; then
			AC_MSG_ERROR([libwebsockets library not found])
		fi
	fi

	AC_MSG_CHECKING([Include non-free minifiers])
	if test "$PHP_NON_FREE" = "yes"; then
		AC_MSG_RESULT([yes])
		phalcon_sources="$phalcon_sources assets/filters/jsminifier.c assets/filters/cssminifier.c "
	else
		AC_MSG_RESULT([no])
		phalcon_sources="$phalcon_sources assets/filters/nojsminifier.c assets/filters/nocssminifier.c "
	fi

	if test "$PHP_STORAGE_WIREDTIGER" = "yes"; then
		AC_MSG_CHECKING([checking libwiredtiger support])
		for i in /usr/local /usr; do
			if test -r $i/include/wiredtiger.h; then
				WIREDTIGER_DIR=$i
				PHP_ADD_INCLUDE($i/include)
				PHP_CHECK_LIBRARY(wiredtiger, wiredtiger_open,
				[
					PHP_ADD_LIBRARY_WITH_PATH(wiredtiger, $i/$PHP_LIBDIR, PHALCON_SHARED_LIBADD)
					AC_DEFINE(PHALCON_USE_WIREDTIGER, 1, [Have wiredtiger support])
					phalcon_sources="$phalcon_sources storage/wiredtiger.c storage/wiredtiger/pack.c storage/wiredtiger/cursor.c cache/backend/wiredtiger.c "
				],[
					AC_MSG_ERROR([Wrong wiredtiger version or library not found])
				],[
					-L$i/$PHP_LIBDIR
				])
				break
			else
				AC_MSG_RESULT([no, found in $i])
			fi
		done

		if test -z "$WIREDTIGER_DIR"; then
			AC_MSG_ERROR([wiredtiger library not found])
		fi
	fi

	if test "$PHP_STORAGE_LMDB" = "yes"; then
		AC_DEFINE(PHALCON_USE_LMDB, 1, [Have lmdb support])
		phalcon_sources="$phalcon_sources storage/lmdb.c storage/lmdb/cursor.c storage/lmdb/mdb.c storage/lmdb/midl.c cache/backend/lmdb.c "
	fi

	if test "$PHP_STORAGE_LIBMDBX" = "yes"; then
		AC_MSG_CHECKING([checking mdbx support])
		for i in /usr/local /usr; do
			if test -r $i/include/mdbx.h; then
				MDBX_DIR=$i
				PHP_ADD_INCLUDE($i/include)
				PHP_CHECK_LIBRARY(mdbx, mdbx_dbi_open,
				[
					PHP_ADD_LIBRARY_WITH_PATH(mdbx, $i/$PHP_LIBDIR, PHALCON_SHARED_LIBADD)
					AC_DEFINE(PHALCON_USE_LIBMDBX, 1, [Have libmdbx support])
					phalcon_sources="$phalcon_sources storage/libmdbx.c storage/libmdbx/cursor.c "
				],[
					AC_MSG_ERROR([Wrong mdbx version or library not found])
				],[
					-L$i/$PHP_LIBDIR
				])
				break
			else
				AC_MSG_RESULT([no, found in $i])
			fi
		done

		if test -z "$MDBX_DIR"; then
			AC_MSG_ERROR([mdbx library not found])
		fi
	fi

	if test "$PHP_STORAGE_LEVELDB" = "yes"; then
		AC_MSG_CHECKING([checking libleveldb support])
		for i in /usr/local /usr; do
			if test -r $i/include/leveldb/c.h; then
				LEVELDB_DIR=$i
				PHP_ADD_INCLUDE($i/include)
				PHP_CHECK_LIBRARY(leveldb, leveldb_open,
				[
					PHP_ADD_LIBRARY_WITH_PATH(leveldb, $i/$PHP_LIBDIR, PHALCON_SHARED_LIBADD)
					AC_DEFINE(PHALCON_USE_LEVELDB, 1, [Have leveldb support])
					phalcon_sources="$phalcon_sources storage/leveldb.c storage/leveldb/writebatch.c storage/leveldb/iterator.c "
				],[
					AC_MSG_ERROR([Wrong leveldb version or library not found])
				],[
					-L$i/$PHP_LIBDIR
				])
				break
			else
				AC_MSG_RESULT([no, found in $i])
			fi
		done

		if test -z "$LEVELDB_DIR"; then
			AC_MSG_ERROR([leveldb library not found])
		fi
	fi

	if test "$PHP_STORAGE_BLOOMFILTER" = "yes"; then
		AC_DEFINE(PHALCON_USE_BLOOMFILTER, 1, [Have bloomfilter support])
		phalcon_sources="$phalcon_sources storage/bloomfilter.c storage/bloomfilter/counting.c "
	fi

	if test "$PHP_STORAGE_DATRIE" = "yes"; then
		AC_DEFINE(PHALCON_USE_DATRIE, 1, [Have datrie support])
		phalcon_sources="$phalcon_sources storage/datrie.c "
	fi

	if test "$PHP_SERVER" = "yes"; then
		AC_MSG_CHECKING([for epoll support])
		AC_TRY_COMPILE(
		[
			#include <sys/epoll.h>
		], [
			int epollfd;
			struct epoll_event e;

			epollfd = epoll_create(1);
			if (epollfd < 0) {
				return 1;
			}

			e.events = EPOLLIN | EPOLLET;
			e.data.fd = 0;

			if (epoll_ctl(epollfd, EPOLL_CTL_ADD, 0, &e) == -1) {
				return 1;
			}

			e.events = 0;
			if (epoll_wait(epollfd, &e, 1, 1) < 0) {
				return 1;
			}
		], [
			AC_DEFINE([PHALCON_USE_SERVER], 1, [Have epoll support])
			AC_MSG_RESULT([yes])
			phalcon_sources="$phalcon_sources server/core.c server.c server/http.c"
		], [
			AC_MSG_RESULT([no])
		])
	fi

	LIBS="$LIBS -pthread -lrt"

	AC_MSG_CHECKING([for shm_open in -pthread -lrt])
	AC_TRY_LINK([
		#include <fcntl.h>
		#include <sys/mman.h>
	], [
		int fp = shm_open("", O_RDWR | O_CREAT | O_EXCL, 0666);
	], [
		AC_DEFINE([PHALCON_USE_SHM_OPEN], 1, [Have shm_open support])
		AC_MSG_RESULT([yes])

		phalcon_sources="$phalcon_sources sync/exception.c sync/mutex.c sync/readerwriter.c sync/event.c sync/semaphore.c sync/sharedmemory.c"
	], [
		AC_MSG_RESULT([shm_open() is not available on this platform])
	])

	AC_CHECK_LIB(c, accept4, AC_DEFINE(HAVE_ACCEPT4, 1, [have accept4]))
	AC_CHECK_LIB(c, signalfd, AC_DEFINE(HAVE_SIGNALFD, 1, [have signalfd]))
	AC_CHECK_LIB(c, poll, AC_DEFINE(HAVE_POLL, 1, [have poll]))
	AC_CHECK_LIB(c, sendfile, AC_DEFINE(HAVE_SENDFILE, 1, [have sendfile]))
	AC_CHECK_LIB(pthread, pthread_rwlock_init, AC_DEFINE(HAVE_RWLOCK, 1, [have pthread_rwlock_init]))
	AC_CHECK_LIB(pthread, pthread_spin_lock, AC_DEFINE(HAVE_SPINLOCK, 1, [have pthread_spin_lock]))
	AC_CHECK_LIB(pthread, pthread_mutex_timedlock, AC_DEFINE(HAVE_MUTEX_TIMEDLOCK, 1, [have pthread_mutex_timedlock]))
	AC_CHECK_LIB(pthread, pthread_barrier_init, AC_DEFINE(HAVE_PTHREAD_BARRIER, 1, [have pthread_barrier_init]))

	PHP_ADD_LIBRARY(pthread)
	PHP_ADD_LIBRARY(dl)
	PHP_CHECK_LIBRARY(rt, clock_gettime, [PHP_ADD_LIBRARY(rt,, PHALCON_SHARED_LIBADD)])

	if test "$PHP_PYTHON" == "yes"; then
		AC_MSG_CHECKING([for python-config support])
		if test "$ext_shared" = "shared" || test "$ext_shared" = "yes"; then
			AC_PATH_PROG([PYTHON_CONFIG],[python-config])
		else
			AC_PATH_PROG([PYTHON_CONFIG],[python-shared-config])
		fi

		if test -z "$PYTHON_CONFIG"; then
			AC_MSG_RESULT(no)
			AC_MSG_ERROR([Cannot find python-config in your system path])
		else
			AC_MSG_RESULT(yes)
		fi

		PYTHON_CFLAGS=`$PYTHON_CONFIG --includes`
		PYTHON_LDFLAGS=`$PYTHON_CONFIG --ldflags`

		CPPFLAGS="${CPPFLAGS} ${PYTHON_CFLAGS}"
		EXTRA_LDFLAGS="${EXTRA_LDFLAGS} ${PYTHON_LDFLAGS}"

		AC_MSG_CHECKING([for python support])

		LIBS="$LIBS ${PYTHON_LDFLAGS}"

		AC_TRY_LINK([
			#include <Python.h>
		],[
			Py_Initialize();
		],[
			AC_DEFINE([PHALCON_USE_PYTHON], 1, [Have python support])
			AC_MSG_RESULT([yes])

			phalcon_sources="$phalcon_sources py/common.c py.c py/object.c py/exception.c py/matplot.c"
		],[
			AC_MSG_ERROR([no])
		])

		AC_CHECK_HEADERS(
			[numpy/arrayobject.h],
			[
				AC_DEFINE([PHALCON_USE_PYTHON_NUMPY], [1], [Have python numpy support])
			],
			,
			[[#include <Python.h>]]
		)

	fi

	PHP_SUBST(PHALCON_SHARED_LIBADD)

	PHP_NEW_EXTENSION(phalcon, $phalcon_sources, $ext_shared)
	PHP_ADD_EXTENSION_DEP([phalcon], [spl])
	PHP_ADD_EXTENSION_DEP([phalcon], [date])

	PHP_C_BIGENDIAN

	PHP_ADD_MAKEFILE_FRAGMENT([Makefile.frag])
fi

PHP_ARG_ENABLE(coverage,  whether to include code coverage symbols,
[  --enable-coverage         Enable code coverage symbols, maintainers only!], no, no)

if test "$PHP_COVERAGE" = "yes"; then
	if test "$GCC" != "yes"; then
		AC_MSG_ERROR([GCC is required for --enable-coverage])
	fi

	case `$php_shtool path $CC` in
		*ccache*[)] gcc_ccache=yes;;
		*[)] gcc_ccache=no;;
	esac

	if test "$gcc_ccache" = "yes" && (test -z "$CCACHE_DISABLE" || test "$CCACHE_DISABLE" != "1"); then
		AC_MSG_ERROR([ccache must be disabled when --enable-coverage option is used. You can disable ccache by setting environment variable CCACHE_DISABLE=1.])
	fi

	lcov_version_list="1.5 1.6 1.7 1.9 1.10"

	AC_CHECK_PROG(LCOV, lcov, lcov)
	AC_CHECK_PROG(GENHTML, genhtml, genhtml)
	PHP_SUBST(LCOV)
	PHP_SUBST(GENHTML)

	if test "$LCOV"; then
		AC_CACHE_CHECK([for lcov version], php_cv_lcov_version, [
			php_cv_lcov_version=invalid
			lcov_version=`$LCOV -v 2>/dev/null | $SED -e 's/^.* //'` #'
			for lcov_check_version in $lcov_version_list; do
				if test "$lcov_version" = "$lcov_check_version"; then
					php_cv_lcov_version="$lcov_check_version (ok)"
				fi
			done
		])
	else
		lcov_msg="To enable code coverage reporting you must have one of the following LCOV versions installed: $lcov_version_list"
		AC_MSG_ERROR([$lcov_msg])
	fi

	case $php_cv_lcov_version in
		""|invalid[)]
			lcov_msg="You must have one of the following versions of LCOV: $lcov_version_list (found: $lcov_version)."
			AC_MSG_ERROR([$lcov_msg])
			LCOV="exit 0;"
		;;
	esac

	if test -z "$GENHTML"; then
		AC_MSG_ERROR([Could not find genhtml from the LCOV package])
	fi

	changequote({,})
	CFLAGS=`echo "$CFLAGS" | $SED -e 's/-O[0-9s]*//g'`
	CXXFLAGS=`echo "$CXXFLAGS" | $SED -e 's/-O[0-9s]*//g'`
	changequote([,])

	CFLAGS="$CFLAGS -O0 --coverage"
	CXXFLAGS="$CXXFLAGS -O0 --coverage"
	EXTRA_LDFLAGS="$EXTRA_LDFLAGS -precious-files-regex \.gcno\\\$$"

	PHP_ADD_MAKEFILE_FRAGMENT([Makefile.frag.coverage])
fi

if test "$GCC" = "yes"; then
	PHP_ADD_MAKEFILE_FRAGMENT([Makefile.frag.deps])
fi
