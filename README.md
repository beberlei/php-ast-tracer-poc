**Experimental Prototype**

For any function:

```php
function foo($a, $b) {
    return $a + $b;
}
```

Use `zend_ast_process` hook to inject the following instrumentation code into the AST:

```php
function foo($a, $b) {
   trace_start();
   try {
       return $a + $b;
   } finally {
       trace_end();
   }
}
```
