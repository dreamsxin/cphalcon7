<?php

class TestResource implements Phalcon\Acl\ResourceAware
{
    protected $user_id;

    protected $resourceName;

    public function __construct($user_id, $resourceName)
    {
        $this->user_id = $user_id;
        $this->resourceName = $resourceName;
    }

    public function getUserId()
    {
        return $this->user_id;
    }

    public function getResourceName()
    {
        return $this->resourceName;
    }
}
