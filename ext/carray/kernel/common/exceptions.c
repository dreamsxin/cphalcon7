#include "php.h"
#include "exceptions.h"
#include "Zend/zend_exceptions.h"

static zend_class_entry * phpsci_ce_CArrayAxisException;
static zend_class_entry * phpsci_ce_CArrayValueErrorException;
static zend_class_entry * phpsci_ce_CArrayTypeErrorException;
static zend_class_entry * phpsci_ce_CArrayOverflowException;
static zend_class_entry * phpsci_ce_CArrayMemoryException;
static zend_class_entry * phpsci_ce_CArrayNotImplementedException;
static zend_class_entry * phpsci_ce_CArrayIndexErrorException;

static const zend_function_entry phpsci_ce_CArrayAxisException_methods[] = {
        PHP_FE_END
};
static const zend_function_entry phpsci_ce_CArrayValueErrorException_methods[] = {
        PHP_FE_END
};
static const zend_function_entry phpsci_ce_CArrayTypeErrorException_methods[] = {
        PHP_FE_END
};

static const zend_function_entry phpsci_ce_CArrayOverflowException_methods[] = {
        PHP_FE_END
};

static const zend_function_entry phpsci_ce_CArrayMemoryException_methods[] = {
        PHP_FE_END
};

static const zend_function_entry phpsci_ce_CArrayNotImplementedException_methods[] = {
        PHP_FE_END
};

static const zend_function_entry phpsci_ce_CArrayIndexErrorException_methods[] = {
        PHP_FE_END
};

/**
 * Initialize Exception Classes
 */
void
init_exception_objects()
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Phalcon", "CArrayAxisException", phpsci_ce_CArrayAxisException_methods);
    phpsci_ce_CArrayAxisException = zend_register_internal_class_ex(&ce, zend_ce_exception);
    INIT_NS_CLASS_ENTRY(ce, "Phalcon", "CArrayValueErrorException", phpsci_ce_CArrayAxisException_methods);
    phpsci_ce_CArrayValueErrorException = zend_register_internal_class_ex(&ce, zend_ce_exception);
    INIT_NS_CLASS_ENTRY(ce, "Phalcon", "CArrayTypeErrorException", phpsci_ce_CArrayAxisException_methods);
    phpsci_ce_CArrayTypeErrorException = zend_register_internal_class_ex(&ce, zend_ce_exception);
    INIT_NS_CLASS_ENTRY(ce, "Phalcon", "CArrayOverflowException", phpsci_ce_CArrayOverflowException_methods);
    phpsci_ce_CArrayTypeErrorException = zend_register_internal_class_ex(&ce, zend_ce_exception);
    INIT_NS_CLASS_ENTRY(ce, "Phalcon", "CArrayMemoryException", phpsci_ce_CArrayMemoryException_methods);
    phpsci_ce_CArrayMemoryException = zend_register_internal_class_ex(&ce, zend_ce_exception);
    INIT_NS_CLASS_ENTRY(ce, "Phalcon", "CArrayNotImplementedException", phpsci_ce_CArrayNotImplementedException_methods);
    phpsci_ce_CArrayNotImplementedException = zend_register_internal_class_ex(&ce, zend_ce_exception);
    INIT_NS_CLASS_ENTRY(ce, "Phalcon", "CArrayIndexErrorException", phpsci_ce_CArrayIndexErrorException_methods);
    phpsci_ce_CArrayIndexErrorException = zend_register_internal_class_ex(&ce, zend_ce_exception);
}

/**
 * Throw CArrayAxisException
 */
void
throw_notimplemented_exception()
{
    zend_throw_exception_ex(phpsci_ce_CArrayNotImplementedException, NOTIMPLEMENTED_EXCEPTION, "%s", 
                            "Whoops! Looks like this situation was unexpected.");
}

/**
 * Throw CArrayAxisException
 */
void
throw_memory_exception(char * msg)
{
    zend_throw_exception_ex(phpsci_ce_CArrayMemoryException, MEMORY_EXCEPTION, "%s", msg);
}

/**
 * Throw CArrayAxisException
 */
void
throw_axis_exception(char * msg)
{
    zend_throw_exception_ex(phpsci_ce_CArrayAxisException, AXIS_EXCEPTION, "%s", msg);
}

/**
 * Throw ValueErrorException
 */
void
throw_valueerror_exception(char * msg)
{
    zend_throw_exception_ex(phpsci_ce_CArrayValueErrorException, VALUEERROR_EXCEPTION, "%s", msg);
}

/**
 * Throw TypeErrorException
 */
void
throw_typeerror_exception(char * msg)
{
    zend_throw_exception_ex(phpsci_ce_CArrayTypeErrorException, TYPEERROR_EXCEPTION, "%s", msg);
}

/**
 * Throw OverflowException
 * @param msg
 */
void
throw_overflow_exception(char * msg)
{
    zend_throw_exception_ex(phpsci_ce_CArrayOverflowException, OVERFLOW_EXCEPTION, "%s", msg);
}

/**
 * Throw IndexErrorException
 * @param msg
 */
void
throw_indexerror_exception(char * msg)
{
    zend_throw_exception_ex(phpsci_ce_CArrayIndexErrorException, INDEXERROR_EXCEPTION, "%s", msg);
}

