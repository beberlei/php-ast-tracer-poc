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

echo (new Targets())->foo(2, 3);
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

echo new Targets()->foo(2, 3);

begin tracing
end tracing
5
