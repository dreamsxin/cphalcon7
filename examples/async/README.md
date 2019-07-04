
## 延迟任务对象 Deferred

* 创建延迟任务
```php
$defer = new Deferred();
```

* 等待延迟任务
```php
Task::await($defer->awaitable());
```

* 发送已完成状态
```php
$defer->resolve('hello');
```

* 发送失败状态
```php
try {
	$defer->resolve('hello');
} catch (\Throwable $e) {
	$defer->fail($e);
}
```

* 生成一个状态为完成的协程对象
```php
Deferred::value('hello');
```

* 生成一个状态为失败的协程对象
```php
Deferred::error(new \Error('Fail'));
```
