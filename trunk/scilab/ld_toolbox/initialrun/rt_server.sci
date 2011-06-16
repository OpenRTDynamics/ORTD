

function SOCKET_open(id,adress,port)
	SOCKET_close(id);
	TCL_EvalStr(["set tclsocket"+string(id)+" [socket "+adress+" "+string(port)+"]";"fconfigure $tclsocket"+string(id)+" -blocking 0 -translation auto"]);
endfunction


function SOCKET_close(id)
	TCL_EvalStr("catch {close $tclsocket"+string(id)+"}");
endfunction

function mat=SOCKET_read(id)
	mat=[];
	cmd=["set tclsocketlong 0;";
	"while {$tclsocketlong >= 0} {";
	"set tclsocketlong [gets $tclsocket"+string(id)+" tclsocketligne];";
	"if {$tclsocketlong >= 0} {";
//	"flush $tclsocket"+string(id)+";";
// bug en cas de ""
	"ScilabEval ""mat=\[\""[string map {\"" \""\""} $tclsocketligne]\"";mat\];""";
//	"ScilabEval ""mat=\[TCL_GetVar(''tclsocketligne'');mat\];""";
	"}";
	"}"];
	TCL_EvalStr(strcat(cmd," "));
	
endfunction


function SOCKET_write(id,commande)
	TCL_EvalStr([	"puts $tclsocket"+string(id)+" """+commande+ascii(10)+"""";
					"flush $tclsocket"+string(id)]);

endfunction