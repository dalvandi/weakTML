<?php

$benchmarks = ["genome", "intruder", "labyrinth", "ssca2", "vacation-low", "vacation-high", "yada"];
$thrds = [1, 2, 4, 8, 16, 32];


function getTime($line)
{
    $t = explode(",", $line)[1];
    $t = preg_replace('/\s+/', '', $t);
    $t = str_replace("Time=", "", $t);
    $t = str_replace("Elapsedtime=", "", $t);
    $t = str_replace("seconds", "", $t);
    $t = str_replace("Timetakenforallis", "", $t);
    $t = str_replace("sec.", "", $t);
    //var_dump($t);
    return $t;
}


function getNoT($line)
{
    $arr = explode(",", $line);
    $not = explode("=", $arr[0]);
    return $not[1];
}

function processTheResult($data, $benchmarks, $thrds)
{
    $res = [];
    foreach ($benchmarks as $bm)
        foreach ($thrds as $t) {
            $c = count($data[$bm][$t]);
            $avg = 0;
            foreach ($data[$bm][$t] as $m)
                $avg += $m;
            //$avg = fdiv($avg, $c);
            $res[$bm][$t] = $avg / $c;
        }

    return $res;
}

function printTheResult($data, $benchmarks, $thrds)
{
    echo "Benchmark, NoT, Time \n";

    foreach ($benchmarks as $bm) {
        foreach ($thrds as $t) {
            echo "{$bm}, {$t}, {$data[$bm][$t]} \n";
        }
    }
}


function generateLatex($data, $benchmarks, $thrds)
{
    echo "Benchmark, NoT, Time \n";

    foreach ($benchmarks as $bm) {
        foreach ($thrds as $t) {
            echo "{$bm}, {$t}, {$data[$bm][$t]} \n";
        }
    }
}


foreach ($benchmarks as $bm) {
    echo "reading benchmark: *** {$bm} ***";
    $handle = fopen("output/{$argv[1]}-{$bm}.txt", "r");
    if ($handle) {
        $c = 0;
        while (($line = fgets($handle)) !== false) {
            $tn = getNoT($line);
            $time = getTime($line);
            $data[$bm][$tn][$c] = floatval($time); // process the line read.
            $c++;
        }
        echo " ... done. \n";
        fclose($handle);
    } else {
        echo "error";
    }
}


$res = processTheResult($data, $benchmarks, $thrds);
//printTheResult($res, $benchmarks, $thrds);
generateLatex($res, $benchmarks, $thrds);
