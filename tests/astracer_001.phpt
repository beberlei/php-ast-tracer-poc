--TEST--
astracer: dump ast
--FILE--
<?php

namespace Testing;

class Targets {
    function foo($a, $b) {
        return $a + $b;
    }
}
--EXPECTF--
namespace Testing;
class Targets {
    public function foo($a, $b) {
        astracer_begin();
        try {
            return $a + $b;
        } finally {
            astracer_end();
        }
    }
}
