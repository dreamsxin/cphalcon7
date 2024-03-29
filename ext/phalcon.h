/*
 * phalcon.h
 *
 *  Created on: 11 січ. 2014
 *      Author: vladimir
 */

#ifndef PHALCON_H
#define PHALCON_H

#include "xhprof.h"

#include "acl.h"
#include "acl/adapter.h"
#include "acl/adapterinterface.h"
#include "acl/adapter/memory.h"
#include "acl/exception.h"
#include "acl/resource.h"
#include "acl/resourceaware.h"
#include "acl/resourceinterface.h"
#include "acl/role.h"
#include "acl/roleaware.h"
#include "acl/roleinterface.h"

#include "annotations/adapter.h"
#include "annotations/adapterinterface.h"
#include "annotations/adapter/apc.h"
#include "annotations/adapter/files.h"
#include "annotations/adapter/memory.h"
#include "annotations/adapter/cache.h"
#include "annotations/annotation.h"
#include "annotations/collection.h"
#include "annotations/exception.h"
#include "annotations/reader.h"
#include "annotations/readerinterface.h"
#include "annotations/reflection.h"

#include "assets/collection.h"
#include "assets/exception.h"
#include "assets/filterinterface.h"
#include "assets/filters/none.h"
#include "assets/filters/cssmin.h"
#include "assets/filters/jsmin.h"
#include "assets/manager.h"
#include "assets/resource.h"
#include "assets/resource/js.h"
#include "assets/resource/css.h"

#include "cache/backend.h"
#include "cache/backendinterface.h"
#include "cache/backend/apc.h"
#include "cache/backend/file.h"
#include "cache/backend/memcached.h"
#include "cache/backend/memory.h"
#include "cache/backend/mongo.h"
#include "cache/backend/redis.h"
#include "cache/backend/yac.h"
#include "cache/backend/wiredtiger.h"
#include "cache/backend/lmdb.h"
#include "cache/exception.h"
#include "cache/frontendinterface.h"
#include "cache/frontend/base64.h"
#include "cache/frontend/data.h"
#include "cache/frontend/igbinary.h"
#include "cache/frontend/json.h"
#include "cache/frontend/none.h"
#include "cache/frontend/output.h"
#include "cache/multiple.h"
#include "cache/yac.h"

#include "pconfig.h"
#include "config/adapter.h"
#include "config/adapterinterface.h"
#include "config/exception.h"
#include "config/adapter/ini.h"
#include "config/adapter/json.h"
#include "config/adapter/php.h"
#include "config/adapter/yaml.h"

#include "crypt.h"
#include "cryptinterface.h"
#include "crypt/exception.h"

#include "profiler.h"
#include "profilerinterface.h"
#include "profiler/item.h"
#include "profiler/iteminterface.h"
#include "profiler/exception.h"

#include "db.h"
#include "db/adapter.h"
#include "db/adapterinterface.h"
#include "db/adapter/pdo.h"
#include "db/adapter/pdo/mysql.h"
#include "db/adapter/pdo/postgresql.h"
#include "db/adapter/pdo/sqlite.h"
#include "db/column.h"
#include "db/columninterface.h"
#include "db/dialect.h"
#include "db/dialectinterface.h"
#include "db/dialect/mysql.h"
#include "db/dialect/postgresql.h"
#include "db/dialect/sqlite.h"
#include "db/exception.h"
#include "db/index.h"
#include "db/indexinterface.h"
#include "db/profiler.h"
#include "db/profiler/item.h"
#include "db/rawvalue.h"
#include "db/reference.h"
#include "db/referenceinterface.h"
#include "db/resultinterface.h"
#include "db/result/pdo.h"
#include "db/builderinterface.h"
#include "db/builder.h"
#include "db/builder/exception.h"
#include "db/builder/where.h"
#include "db/builder/join.h"
#include "db/builder/select.h"
#include "db/builder/update.h"
#include "db/builder/insert.h"
#include "db/builder/delete.h"

#include "debug.h"
#include "debug/exception.h"
#include "debug/dump.h"

#include "di.h"
#include "diinterface.h"
#include "di/exception.h"
#include "di/factorydefault.h"
#include "di/factorydefault/cli.h"
#include "di/injectionawareinterface.h"
#include "di/injectable.h"
#include "di/service.h"
#include "di/serviceinterface.h"
#include "di/service/builder.h"

#include "dispatcher.h"
#include "dispatcherinterface.h"

#include "escaper.h"
#include "escaperinterface.h"
#include "escaper/exception.h"

#include "events/event.h"
#include "events/eventinterface.h"
#include "events/eventsawareinterface.h"
#include "events/exception.h"
#include "events/managerinterface.h"
#include "events/manager.h"
#include "events/listener.h"

#include "exception.h"
#include "continueexception.h"
#include "exitexception.h"

#include "filter.h"
#include "filterinterface.h"
#include "filter/exception.h"
#include "filter/userfilterinterface.h"

#include "flash.h"
#include "flashinterface.h"
#include "flash/direct.h"
#include "flash/exception.h"
#include "flash/session.h"

#include "forms/element.h"
#include "forms/elementinterface.h"
#include "forms/element/check.h"
#include "forms/element/date.h"
#include "forms/element/email.h"
#include "forms/element/file.h"
#include "forms/element/hidden.h"
#include "forms/element/numeric.h"
#include "forms/element/password.h"
#include "forms/element/radio.h"
#include "forms/element/select.h"
#include "forms/element/submit.h"
#include "forms/element/text.h"
#include "forms/element/textarea.h"
#include "forms/exception.h"
#include "forms/form.h"
#include "forms/manager.h"

#include "http/parser.h"
#include "http/cookie.h"
#include "http/cookie/exception.h"
#include "http/request.h"
#include "http/requestinterface.h"
#include "http/request/exception.h"
#include "http/request/file.h"
#include "http/request/fileinterface.h"
#include "http/response.h"
#include "http/responseinterface.h"
#include "http/response/cookies.h"
#include "http/response/cookiesinterface.h"
#include "http/response/exception.h"
#include "http/response/headers.h"
#include "http/response/headersinterface.h"
#include "http/uri.h"
#include "http/client.h"
#include "http/client/exception.h"
#include "http/client/header.h"
#include "http/client/response.h"
#include "http/client/adapter.h"
#include "http/client/adapterinterface.h"
#include "http/client/adapter/curl.h"
#include "http/client/adapter/stream.h"

#include "image.h"
#include "image/vips.h"
#include "image/adapter.h"
#include "image/adapterinterface.h"
#include "image/adapter/gd.h"
#include "image/adapter/imagick.h"
#include "image/exception.h"

#include "kernel.h"

#include "loader.h"
#include "loader/exception.h"

#include "logger.h"
#include "logger/adapter.h"
#include "logger/adapterinterface.h"
#include "logger/adapter/file.h"
#include "logger/adapter/firephp.h"
#include "logger/adapter/stream.h"
#include "logger/adapter/syslog.h"
#include "logger/adapter/direct.h"
#include "logger/exception.h"
#include "logger/formatter.h"
#include "logger/formatterinterface.h"
#include "logger/formatter/firephp.h"
#include "logger/formatter/json.h"
#include "logger/formatter/line.h"
#include "logger/formatter/syslog.h"
#include "logger/item.h"
#include "logger/multiple.h"

#include "user/component.h"
#include "user/module.h"
#include "user/plugin.h"
#include "user/logic.h"

#include "application.h"
#include "application/exception.h"

#include "router.h"
#include "routerinterface.h"
#include "router/exception.h"

#include "cli/console.h"
#include "cli/console/exception.h"
#include "cli/dispatcher.h"
#include "cli/dispatcher/exception.h"
#include "cli/router.h"
#include "cli/router/exception.h"
#include "cli/task.h"
#include "cli/options.h"
#include "cli/options/exception.h"
#include "cli/color.h"

#include "mvc/application.h"
#include "mvc/application/exception.h"
#include "mvc/controller.h"
#include "mvc/controllerinterface.h"
#include "mvc/dispatcher.h"
#include "mvc/dispatcherinterface.h"
#include "mvc/dispatcher/exception.h"
#include "mvc/micro.h"
#include "mvc/micro/collection.h"
#include "mvc/micro/collectioninterface.h"
#include "mvc/micro/exception.h"
#include "mvc/micro/lazyloader.h"
#include "mvc/micro/middlewareinterface.h"
#include "mvc/orm.h"
#include "mvc/model.h"
#include "mvc/modelinterface.h"
#include "mvc/model/behavior.h"
#include "mvc/model/behaviorinterface.h"
#include "mvc/model/behavior/softdelete.h"
#include "mvc/model/behavior/timestampable.h"
#include "mvc/model/criteria.h"
#include "mvc/model/criteriainterface.h"
#include "mvc/model/exception.h"
#include "mvc/model/manager.h"
#include "mvc/model/managerinterface.h"
#include "mvc/model/metadata.h"
#include "mvc/model/metadatainterface.h"
#include "mvc/model/metadata/apc.h"
#include "mvc/model/metadata/files.h"
#include "mvc/model/metadata/memory.h"
#include "mvc/model/metadata/session.h"
#include "mvc/model/metadata/memcached.h"
#include "mvc/model/metadata/redis.h"
#include "mvc/model/metadata/mongo.h"
#include "mvc/model/metadata/cache.h"
#include "mvc/model/metadata/strategy/annotations.h"
#include "mvc/model/metadata/strategy/introspection.h"
#include "mvc/model/query.h"
#include "mvc/model/queryinterface.h"
#include "mvc/model/query/exception.h"
#include "mvc/model/query/builder.h"
#include "mvc/model/query/builderinterface.h"
#include "mvc/model/query/builder/where.h"
#include "mvc/model/query/builder/join.h"
#include "mvc/model/query/builder/select.h"
#include "mvc/model/query/builder/update.h"
#include "mvc/model/query/builder/insert.h"
#include "mvc/model/query/builder/delete.h"
#include "mvc/model/query/lang.h"
#include "mvc/model/query/status.h"
#include "mvc/model/query/statusinterface.h"
#include "mvc/model/relation.h"
#include "mvc/model/relationinterface.h"
#include "mvc/model/resultinterface.h"
#include "mvc/model/resultset.h"
#include "mvc/model/resultsetinterface.h"
#include "mvc/model/resultset/complex.h"
#include "mvc/model/resultset/simple.h"
#include "mvc/model/row.h"
#include "mvc/model/transaction.h"
#include "mvc/model/transactioninterface.h"
#include "mvc/model/transaction/exception.h"
#include "mvc/model/transaction/failed.h"
#include "mvc/model/transaction/manager.h"
#include "mvc/model/transaction/managerinterface.h"
#include "mvc/model/validationfailed.h"
#include "mvc/moduledefinitioninterface.h"
#include "mvc/router.h"
#include "mvc/routerinterface.h"
#include "mvc/router/annotations.h"
#include "mvc/router/exception.h"
#include "mvc/router/group.h"
#include "mvc/router/route.h"
#include "mvc/router/routeinterface.h"
#include "mvc/url.h"
#include "mvc/urlinterface.h"
#include "mvc/url/exception.h"
#include "mvc/user/component.h"
#include "mvc/user/module.h"
#include "mvc/user/plugin.h"
#include "mvc/user/logic.h"
#include "mvc/user/logic/model.h"
#include "mvc/view.h"
#include "mvc/viewinterface.h"
#include "mvc/view/engine.h"
#include "mvc/view/engineinterface.h"
#include "mvc/view/engine/php.h"
#include "mvc/view/exception.h"
#include "mvc/view/simple.h"
#include "mvc/view/model.h"
#include "mvc/view/modelinterface.h"

#include "paginator/adapterinterface.h"
#include "paginator/adapter.h"
#include "paginator/adapter/model.h"
#include "paginator/adapter/nativearray.h"
#include "paginator/adapter/querybuilder.h"
#include "paginator/adapter/dbbuilder.h"
#include "paginator/adapter/sql.h"
#include "paginator/exception.h"

#include "queue/beanstalk.h"
#include "queue/beanstalk/job.h"

#include "registry.h"
#include "random.h"

#include "security.h"
#include "security/random.h"
#include "security/exception.h"

#include "session/adapter.h"
#include "session/adapterinterface.h"
#include "session/adapter/files.h"
#include "session/bag.h"
#include "session/baginterface.h"
#include "session/exception.h"
#include "session/adapter/memcached.h"
#include "session/adapter/cache.h"

#include "tag.h"
#include "tag/exception.h"
#include "tag/select.h"

#include "translate/adapter.h"
#include "translate/adapterinterface.h"
#include "translate/exception.h"
#include "translate/adapter/nativearray.h"
#include "translate/adapter/gettext.h"
#include "translate/adapter/php.h"

#include "text.h"
#include "arr.h"
#include "files.h"
#include "matrix.h"
#include "date.h"
#include "date/datetime.h"

#include "binary.h"
#include "binary/exception.h"
#include "binary/reader.h"
#include "binary/writer.h"

#include "socket.h"
#include "socket/client.h"
#include "socket/server.h"
#include "socket/exception.h"

#include "websocket/connection.h"
#include "websocket/server.h"
#include "websocket/client.h"
#include "websocket/eventloopinterface.h"

#include "process/sharedmemory.h"

#include "intrusive/avltree.h"
#include "intrusive/avltree/node.h"
#include "intrusive/rbtree.h"
#include "intrusive/rbtree/node.h"

#include "validationinterface.h"
#include "validation.h"
#include "validation/exception.h"
#include "validation/message.h"
#include "validation/messageinterface.h"
#include "validation/message/group.h"
#include "validation/validator.h"
#include "validation/validatorinterface.h"
#include "validation/validator/between.h"
#include "validation/validator/confirmation.h"
#include "validation/validator/email.h"
#include "validation/validator/exclusionin.h"
#include "validation/validator/identical.h"
#include "validation/validator/inclusionin.h"
#include "validation/validator/presenceof.h"
#include "validation/validator/regex.h"
#include "validation/validator/stringlength.h"
#include "validation/validator/url.h"
#include "validation/validator/file.h"
#include "validation/validator/numericality.h"
#include "validation/validator/json.h"
#include "validation/validator/uniqueness.h"
#include "validation/validator/alnum.h"
#include "validation/validator/alpha.h"
#include "validation/validator/digit.h"
#include "validation/validator/date.h"

#include "version.h"

#include "chart/qrcode.h"
#include "chart/captcha.h"
#include "chart/captcha/tiny.h"
#include "chart/exception.h"

#include "async.h"
#include "async/core.h"

#include "thread/exception.h"
#include "thread/pool.h"

#include "sync/exception.h"
#include "sync/mutex.h"
#include "sync/readerwriter.h"
#include "sync/semaphore.h"
#include "sync/event.h"
#include "sync/sharedmemory.h"

#include "process/proc.h"
#include "process/exception.h"

#include "snowflake.h"

#include "storage/exception.h"
#include "storage/frontendinterface.h"
#include "storage/frontend/base64.h"
#include "storage/frontend/igbinary.h"
#include "storage/frontend/json.h"
#include "storage/btree.h"
#include "storage/wiredtiger.h"
#include "storage/wiredtiger/cursor.h"
#include "storage/bloomfilter.h"
#include "storage/bloomfilter/counting.h"
#include "storage/datrie.h"
#include "storage/lmdb.h"
#include "storage/lmdb/cursor.h"
#include "storage/libmdbx.h"
#include "storage/libmdbx/cursor.h"
#include "storage/leveldb.h"
#include "storage/leveldb/iterator.h"
#include "storage/leveldb/writebatch.h"

#include "server.h"
#include "server/exception.h"
#include "server/http.h"
#include "server/simple.h"

#include "py/common.h"
#include "py.h"
#include "py/object.h"
#include "py/exception.h"
#include "py/matplot.h"

#include "aop.h"
#include "aop/exception.h"
#include "aop/lexer.h"
#include "aop/joinpoint.h"

#include "num.h"
#include "num/ndarray.h"

#include "jwt.h"

#endif /* PHALCON_H */
