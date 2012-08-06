del TestPresence.cab
cabarc -s 6144 N TestPresence.cab TestPresence.ocx TestPresence.inf
..\..\Utils\Sign\signcode -n "SIL Software Evaluator" -spc D:\Comodo.spc -v D:\Comodo.pvk -t http://timestamp.comodoca.com/authenticode TestPresence.cab
