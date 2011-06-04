function [x,y,typ] = filewriter(job,arg1,arg2)

  x=[];y=[];typ=[];
  select job
  case 'plot' then
    exprs=arg1.graphics.exprs;
    standard_draw(arg1)
  case 'getinputs' then
    [x,y,typ]=standard_inputs(arg1)
  case 'getoutputs' then
    [x,y,typ]=standard_outputs(arg1)
  case 'getorigin' then
    [x,y]=standard_origin(arg1)
  case 'set' then
    x=arg1
    model=arg1.model;graphics=arg1.graphics;
    exprs=graphics.exprs;
    while %t do
  try
  getversion('scilab');
      [ok,veclen,fname,exprs]=..
      scicos_getvalue('Filewriter',..
         ['Input length:';
          'Filename :'],..
      list('vec',1,'str',-1),exprs)
catch
      [ok,veclen,fname,exprs]=..
      getvalue('Filewriter',..
         ['Input length:';
          'Filename :'],..
      list('vec',1,'str',-1),exprs)
end;
     if ~ok then break,end
      in=[[veclen]]
      out=[model.out]
      evtin=[1]
      evtout=[]
      [model,graphics,ok]=check_io(model,graphics,in,out,evtin,evtout);
      if ok then
        graphics.exprs=exprs;
        model.ipar=[1;
veclen;
1;
1;
length(ascii(fname));
ascii(fname)'

];
        model.rpar=[];
   model.dstate=[1];
        x.graphics=graphics;x.model=model
        break
      end
    end
  case 'define' then
     veclen=1;
     fname='file.dat';
   model=scicos_model()
   model.sim=list('rt_filewriter',4)
   model.in=[[veclen]]
   model.out=[]
   model.evtin=[1]
   model.evtout=[]
   model.ipar=[1;
veclen;
1;
1;
length(ascii(fname));
ascii(fname)'

];
   model.rpar=[];
 model.dstate=[1];
 model.blocktype='d';
 model.dep_ut=[%f %f];
    exprs=[sci2exp(veclen);fname]
    gr_i=['xstringb(orig(1),orig(2),[''Realtime Filewriter''  ],sz(1),sz(2),''fill'');'];
    x=standard_define([4 2],model,exprs,gr_i)
case 'readout' then
      BLOCK.version=020;
      BLOCK.name='filewriter';
      BLOCK.comp_name='rt_filewriter';
      BLOCK.desr_short='Filewriter';
      BLOCK.dep_u=%f;
      BLOCK.dep_t=%f;
      BLOCK.blocktype='d';
      BLOCK.dstate='1';
      BLOCK.IOmatrix=%f;
      BLOCK.inset=%t;
      BLOCK.in='[veclen]';
      BLOCK.outset=%f;
      BLOCK.out='';
      BLOCK.evtin='1';
      BLOCK.evtout='';
      BLOCK.size='4 2';
      BLOCK.completelabel=%f;
      BLOCK.label=[39,39,82,101,97,108,116,105,109,101,32,70,105,108,101,119,114,105,116,101,114,39,39,10,10];
      BLOCK.ipar=[49,59,10,118,101,99,108,101,110,59,10,49,59,10,49,59,10,108,101,110,103,116,104,40,97,..
         115,99,105,105,40,102,110,97,109,101,41,41,59,10,97,115,99,105,105,40,102,110,97,109,101,..
         41,39,10,10];
      BLOCK.rpar=[];
      BLOCK.opar=[];
      BLOCK.parameter=list();
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(1).name='veclen';
      BLOCK.parameter(1).text='Input length:';
      BLOCK.parameter(1).type='vec';
      BLOCK.parameter(1).size='1';
      BLOCK.parameter(1).init='1';
      BLOCK.parameter(1).visible_plot=%f;
      BLOCK.parameter($+1)=[];
      BLOCK.parameter(2).name='fname';
      BLOCK.parameter(2).text='Filename :';
      BLOCK.parameter(2).type='str';
      BLOCK.parameter(2).size='-1';
      BLOCK.parameter(2).init='file.dat';
      BLOCK.parameter(2).visible_plot=%f;
      x=BLOCK;
  end
endfunction
