<?php

class TestRole implements Phalcon\Acl\RoleAware
{
    protected $id;
    protected $roleName;
    public function __construct($id,$roleName)
    {
        $this->id = $id;
        $this->roleName = $roleName;
    }
    public function getId()
    {
        return $this->id;
    }
    public function getRoleName()
    {
        return $this->roleName;
    }
}
