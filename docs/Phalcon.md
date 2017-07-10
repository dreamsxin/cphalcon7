# Phalcon7 beginner's handbook

*Author: Dreamsxin*

## Benefits

- The Fastest
- Few Memory Usage
- Cross-Platform
- Highly Decoupled
- Full-stack Framework

## Startup Process

	1.Register an autoloader
	2.Create a DI - Phalcon\Di\FactoryDefault
	3.Setup the view component
	4.Setup a base URI
	5.Handle the request - Phalcon\Mvc\Application::handle
		5.1.Fire event – application:boot
		5.2.Handle the URI pattern (if any)
		5.3.Process the module definition (if any)
			5.3.1.Fire event  - application:beforeStartModule
			5.3.2.Call method registerAutoloaders
			5.3.3.Call method registerServices
			5.3.3.Fire event - application:afterStartModule
		5.4.Fire event – application:beforeHandleRequest
		5.5.Dispatch – Phalcon\Mvc\Dispatcher::dispatch → Phalcon\Dispatcher::dispatch
			5.5.1.Fire event - dispatch:beforeDispatchLoop
			5.5.2.Dispatch loop
				5.5.2.1.Fire event - dispatch:beforeDispatch
				5.5.2.2.Create the complete controller class name
				5.5.2.3.Load controller class
				5.5.2.3.Check if the action exists in the controller
				5.5.2.4.Fire event - dispatch:beforeExecuteRoute
				5.5.2.5.Call the controller method beforeExecuteroute
				5.5.2.5.Call the controller method initialize
				5.5.2.4.Fire event - dispatch:afterInitialize
				5.5.2.5.Call the action
				5.5.2.6.Fire event - dispatch:afterExecuteRoute
				5.5.2.7.Fire event  - dispatch:afterDispatch
				5.5.2.8.Call the controller method afterExecuteroute
				5.5.2.9.Fire event - dispatch:afterDispatchLoop
				5.5.2.10.Retrun the controller
			5.6.Fire event – application:afterHandleRequest
			5.7.Rendering Views
				5.7.1.Fire event - application:beforeRenderView
				5.7.2.Call the controller method beforeRenderView
				5.7.3.Call method – Phalcon\Mvc\View::render
					5.7.3.1.Load the template engines
					5.7.3.2.Fire event  - view:beforeRender
					5.7.3.3.Hierarchical Rendering
						5.7.3.3.1.Render action view
						5.7.3.3.2.Render before layout view
						5.7.3.3.3.Render controller layout view
						5.7.3.3.4.Render namespace layout view
						5.7.3.3.5.Render after layout view
						5.7.3.3.6.Render main view
					5.7.3.4.Fire event - view:afterRender
				5.7.4.Fire event - application:afterRenderView
				5.7.5.Call the controller method afterRenderView
			5.8.Fire event - application:beforeSendResponse
			5.9.Get the processed content - Phalcon\Mvc\View::getContent
			5.10.Send headers - Phalcon\Http\Response::sendHeaders
			5.11.Send cookies – Phalcon\Http\Response::sendCookies
			5.12.Return the response

## Events

### Application Events

- boot
- beforeStartModule
- afterStartModule
- beforeHandleRequest
- afterHandleRequest
- beforeRenderView
- afterRenderView
- beforeSendResponse
- afterSendResponse

### Router Events

- beforeCheckRoutes
- beforeCheckRoute
- matchedRoute
- notMatchedRoute
- afterCheckRoutes

### Dispatch Events

- beforeDispatchLoop
- beforeDispatch
- beforeNotFoundAction
- beforeExecuteRoute
- afterInitialize
- afterExecuteRoute
- afterDispatch
- afterDispatchLoop

### Load Events

- beforeCheckPath
- pathFound
- beforeCheckClass
- afterCheckClass

### Dependency Injection Events

- beforeServiceResolve
- afterServiceResolve

### Models Events

- beforeOperation
- beforeQuery
- afterQuery
- notDeleted
- notSaved
- onValidationFails
- beforeValidation
- beforeValidationOnCreate
- beforeValidationOnUpdate
- validation
- afterValidationOnCreate
- afterValidationOnUpdate
- afterValidation
- beforeSave
- beforeCreate
- beforeUpdate
- beforeDelete
- afterCreate
- afterUpdate
- afterDelete
- afterSave
- notSave
- afterOperation
- afterToArray

### View Events

- beforeRender
- beforeRenderView
- notFoundView
- afterRenderView
- afterRender

## Events Trigger A Process

* Event format: event type + `:` + event name

### General Case

Any class Inherited from `Phalcon\Di\Injectable`.

	1.Call method `fireEvent` or `fireEventCancel`
	2.Check if there is a method with the same name of the event, if yes call the method
	3.Send a notification to the events manager - Phalcon\Events\Manager::fire
	4.Internal handler to call a queue of events - Phalcon\Events\Manager::fireQueue
	5.Check if the listener is a closure or has implemented an event with the same name

### Way 2 Case

`Views` use this way.

	1.Send a notification to the events manager - Phalcon\Events\Manager::fire
	2.Internal handler to call a queue of events - Phalcon\Events\Manager::fireQueue
	3.Check if the listener is a closure or has implemented an event with the same name

### Way 3 Case

`Models` use this way.

	1.Call self manager method `notifyEvent`
	2.Send a notification to the events manager - Phalcon\Events\Manager::fire
	3.Internal handler to call a queue of events - Phalcon\Events\Manager::fireQueue
	4.Check if the listener is a closure or has implemented an event with the same name

## Model

### Query records

	1.Creates a query builder - Phalcon\Mvc\Model\Manager::createBuilder
	2.Fire event - model:beforeQuery
	3.Gets the query - Phalcon\Mvc\Model\Query\Builder::getQuery
	4.Execute the query
	5.Fire event - model:afterQuery

## Design pattern

	1.Factory Method - Phalcon\Mvc\Model\Manager::createBuilder
	2.Prototype - Models Clone
	3.Adapter - DB, Image etc
	4.Bridge - DI
	5.Strategy
	6.Template Method
	7.Observer - Events
	8.Iterator