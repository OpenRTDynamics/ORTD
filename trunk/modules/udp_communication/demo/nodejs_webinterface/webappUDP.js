/*

  Sample communication interface to ORTD using UDP datagrams.
  A web-interface is provided, go to http://localhost:8090
  UDPio.sce is the counterpart whose simulation can be controlled
  via the web-interface.
  The web-interface is defined in html/main.html
  
*/

var app = require('http').createServer(handler);
var io = require('socket.io').listen(app); io.set('log level', 1);
var fs = require('fs');
var dgram = require('dgram');

// http-interface
app.listen(8090);

// UDP config
var PORT = 10000;
var HOST = '127.0.0.1';
var ORTD_HOST = '127.0.0.1'; // the IP and port of the ORTD simulator running UDPio.sce
var ORTD_PORT = 10001;

var NValues = 2; // must be the same as NValues_send defined in UDPio.sce when calling UDPSend
var DataBufferSize = 20000; // Number of elementes stored in the ringbuffer
var NParameters = 2;

// 
// data ringbuffer
// 



var RingBuffer = new RingBuffer(DataBufferSize, NValues);
console.log("RingBuffer created");

// 
// http-server
// 
function handler (req, res) {
  fs.readFile('html/main_StaticPlot.html',
  function (err, data) {
    if (err) {
      res.writeHead(500);
      return res.end('Error loading main.html');
    }
 
    res.writeHead(200);
    res.end(data);
  });
}
 
 
  
//  
// UDP interface
// 
var server = dgram.createSocket('udp4');
server.on('listening', function () {
    var address = server.address();
    console.log('UDP Server listening on ' + address.address + ":" + address.port);
});


// Buffer for sending UDP packets
var UDPSendPacketBuffer = new Buffer(2000); // size is propably bigger than every UDP-Packet


server.on('message', function (message, remote) {
    // received new packet from ORTD via UDP
    //console.log(remote.address + ':' + remote.port);  
    var i;
    
    try {
      // disassemble header
      var SenderID = message.readInt32LE( 0 );
      var PacketCounter = message.readInt32LE( 4 );
      var SourceID = message.readInt32LE( 8 );

      // check wheter the sender ID is correct
      if (SenderID != 1295793)
	throw 1;
      
      // check if the recved packet has the correct size
      if ( message.length != 12+8*NValues) 
	throw 2;

      // disassemble data-values
      var ValuesBuffer = message.slice(12, 12+8*NValues);
      var Values = new Array(NValues);
      
      for (i=0; i<NValues; ++i)
	Values[i] = ValuesBuffer.readDoubleLE( i*8 );
	
      // send to all web browser
      try { io.sockets.emit('Values', Values ); } catch(err) { }
	  
      // buffer the vales
      RingBuffer.addElement(Values);
    } catch(err) {
      console.log("Received a malformed UDP-packet");
    }
});
 
// bind UDP-socket
server.bind(PORT, HOST);


// 
// websockets connection to the web browser(s) 
// 

io.sockets.on('connection', function (socket) {
  console.log('new socket.io connection');
  
  // Wait for requirest to send the entire ringbuffer
  socket.on('GetBuffer', function (data) {
    try { socket.emit('GetBufferReturn', [ RingBuffer.WriteIndex , RingBuffer.DataBuffer ] ); } catch(err) { }
  });
  
  // wait for a parameter upload by the client
  socket.on('ChangeParam_Set', function (data) {
    //
    // assemble the binary udp-packet
    //
    
    var i;
    
    // the required message length
    var MessageLength = 12+NParameters*8;
    
    // write the header of the UDP-packet
    UDPSendPacketBuffer.writeInt32LE( 1, 0 );
    UDPSendPacketBuffer.writeInt32LE( 1234, 4 );
    UDPSendPacketBuffer.writeInt32LE( 6468235, 8 );
    
    // add the parameters given in data[i]
    for (i=0; i<NParameters; ++i) {
      UDPSendPacketBuffer.writeDoubleLE(  data[i], 12+i*8 );
    }
    		
    // send this packet to ORTD
    server.send(UDPSendPacketBuffer, 0, MessageLength, ORTD_PORT, ORTD_HOST, function(err, bytes) {
      if (err) throw err;
      console.log('UDP message sent to ' + ORTD_HOST +':'+ ORTD_PORT);
    });    
  });
  
});


// 
// ring buffer class for storing data send by ORTD
// 
function RingBuffer(DataBufferSize,NumElements)
{
  this.DataBufferSize=DataBufferSize;
  this.NumElements=NumElements;

  this.DataBuffer = CreateDataBufferMultidim(DataBufferSize, NumElements);
  this.WriteIndex = 0;
  
  function CreateDataBufferMultidim(DataBufferSize, NumElements) {
    var DataBuffer = new Array(DataBufferSize);
    var i;
    var j;
    
    for (i=0; i<DataBufferSize; ++i) {
      DataBuffer[i] = new Array(NumElements);
      
      for (j=0; j<NumElements; ++j) {
	  DataBuffer[i][j] = 0;
      }
    }
    return DataBuffer;
  }
  
  this.addElement=addElement;
  function addElement(Values)  {
    // console.log("adding element at index " + this.WriteIndex);
    var i;
    for (i = 0; i<this.NumElements; ++i) {
	this.DataBuffer[this.WriteIndex][i] = Values[i]; // copy data 	
    }
    
    // inc counter
    this.WriteIndex++;
    
    // wrap counter
    if (this.WriteIndex >= this.DataBufferSize) {
      this.WriteIndex = 0;
    }
  }
}
