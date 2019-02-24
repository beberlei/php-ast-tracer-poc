<?php

namespace Testing;

class Targets {

    function foo($a, $b) {
        return $a + $b;
    }

    function bar($a, $b) {
        trace_start();
        try {
            return $a + $b;
        } finally {
            trace_end();
        }
    }
}


require_once __DIR__ ."/../../nikic/php-ast/util.php";
$ast = \ast\parse_code(file_get_contents(__FILE__), $version=50);
var_dump($ast);
echo ast_dump($ast), "\n";
