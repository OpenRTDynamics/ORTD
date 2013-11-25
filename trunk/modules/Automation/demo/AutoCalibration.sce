// TODO: Make this working

function [sim, q, R, g_s] = CalibrationExample(sim, AccGyro, Flag, Ts)


  function [sim, finished, outlist, userdata] = experiment(sim, ev, inlist, userdata)
      // Do the experiment
      
      AccGyro = inlist(1);
      [sim] = ld_printf(sim, 0, AccGyro, "Collecting data ... ", 6);

      // Store the sensor data into a shared memory
      [sim, Six] = ld_const(sim, ev, 6);
      [sim, zero] = ld_const(sim, ev, 0);
      [sim, writeI] = ld_counter(sim, 0, count=Six, reset=zero, resetto=zero, initial=1);

      [sim] = ld_write_global_memory(sim, 0, data=AccGyro, index=writeI, ...
                                     ident_str="IMU_EKF_SensorData", datatype=ORTD.DATATYPE_FLOAT, ...
                                     ElementsToWrite=6);

 
      // wait until a number of time steps to be passed, then tell ld_AutoExperiment that
      // the experiment has finished.
      [sim, finished] = ld_steps2(sim, ev, activation_simsteps=NcalibSamples, values=[0,1] );
//       [sim, finished] = ld_play_simple(sim, ev, [ zeros(NcalibSamples, 1); 1 ]);

      [sim, out] = ld_const(sim, ev, 0);
      outlist=list(out);
  endfunction

  function [sim, outlist, userdata] = whileComputing(sim, ev, inlist, userdata)
      // While the computation is running this is called regularly
      [sim, out] = ld_const(sim, ev, 0);
      outlist=list(out);
  endfunction

  function [sim, outlist, userdata] = whileIdle(sim, ev, inlist, userdata)
      AccGyro = inlist(1);

      // Do wait
      [sim, readI] = ld_const(sim, ev, 1); // start at index 1
      [sim, Calibration] = ld_read_global_memory(sim, ev, index=readI, ident_str="IMU_EKF_CalibrationResult", ...
                                                  datatype=ORTD.DATATYPE_FLOAT, ...
                                                  ElementsToRead=20);

//       [sim] = ld_printf(sim, 0, Calibration, "The calibration result is ", 20);


      //  Here, the EKF is implemented



      
      [sim, out] = ld_const(sim, ev, 0);
      outlist=list(out);
  endfunction

  function [sim, CalibrationOk, userdata] = evaluation(sim, userdata)
      // This superblock will run the evaluation of the experiment in a thread.
      // The superblock describes a sub-simulation, whereby only one step is simulated
      // which is enough to call scilab one signle time
      
      ev = 0;

      // define a Scilab function that performs the calibration
      function [block]=scilab_comp_fn( block, flag )
	// This scilab function is called during run-time
	// NOTE: Please note that the variables defined outside this
	//       function are typically not available at run-time.
	//       This also holds true for self defined Scilab functions!

        NSensors = 6;  NcalibSamples = 500;

	function outvec=PerformCalibration(accX, accY, accZ, gyrX, gyrY, gyZ)
	  function [y]=CrossProduct(vp, vm)
	    y(1) = vp(2)*vm(3) - vp(3)*vm(2);
	    y(2) = vp(3)*vm(1) - vp(1)*vm(3);
	    y(3) = vp(1)*vm(2) - vp(2)*vm(1);
	  endfunction
	  function [y] = UnitVector(x)
	    y = x / sqrt(sum(x.*x));
	  endfunction
	  function [l] = VectorNorm(x)
	    l = sqrt(sum(x.*x));
	  endfunction
	  function [q] = RotmatToQuaternion( R )
	    q = zeros(4,1);
	    t = sqrt( R(1,1) + R(2,2) + R(3,3) + 1 );
	    q(1) = 1/2 * t;
	    q(2) = 1/2 * sign( R(3,2) - R(2,3) ) * sqrt( R(1,1) - R(2,2) - R(3,3) + 1 );
	    q(3) = 1/2 * sign( R(1,3) - R(3,1) ) * sqrt( R(2,2) - R(3,3) - R(1,1) + 1 );
	    q(4) = 1/2 * sign( R(2,1) - R(1,2) ) * sqrt( R(3,3) - R(1,1) - R(2,2) + 1 );
	  endfunction
	  function Q=Qq(q)
	    q0=q(1);   q1=q(2);   q2=q(3);   q3=q(4);
	    Q = [2*(q0^2+q1^2) - 1  2*(q1*q2-q0*q3)    2*(q1*q3+q0*q2);
		  2*(q1*q2+q0*q3)    2*(q0^2+q2^2) - 1  2*(q2*q3-q0*q1);
		  2*(q1*q3-q0*q2)    2*(q2*q3+q0*q1)    2*(q0^2+q3^2) - 1];
	  endfunction

          outvec = zeros(20,1);

	  printf("Calibrating...\n");

          means=[ mean(accX); mean(accY); mean(accZ); mean(gyrX);  mean(gyrY);  mean(gyrZ) ];
          CalibOk = 1;

 	  printf("The means are:\n"); disp(means);

          // norm of gS becomes the z-Axis of Earth-frame 
	  gS = means(1:3);
	  b1S = UnitVector(gS);
	  b2S = UnitVector( CrossProduct( b1S, [ 1;0;0 ] ) );
	  b3S = UnitVector( CrossProduct( b1S, b2S ) );
	  R = [ b3S, -b2S, b1S ];
	  printf("det(R)\n"); det(R) 
	  printf("disp(R''*R)\n"); disp(R'*R);

	  [q] = RotmatToQuaternion( R ); sum(q.^2)
	  R_ = Qq(q);

          printf("q\n"); disp(q);
          printf("R\n"); disp(R);
          printf("R_\n"); disp(R_);

          // pack
          outvec(1) = CalibOk;
          outvec(2:7) = means(:);
          outvec(8:11) = q(:);
	endfunction

	select flag

	  case 1 // output
            // split the sensor data
            data = block.inptr(1);

            accX = zeros(NcalibSamples,1);
            accY = zeros(NcalibSamples,1);
            accZ = zeros(NcalibSamples,1);
            gyrX = zeros(NcalibSamples,1);
            gyrY = zeros(NcalibSamples,1);
            gyrZ = zeros(NcalibSamples,1);

            for i=1:NcalibSamples
              accX(i) = data( (i-1)*NSensors + 1 );
              accY(i) = data( (i-1)*NSensors + 2 );
              accZ(i) = data( (i-1)*NSensors + 3 );

              gyrX(i) = data( (i-1)*NSensors + 4 );
              gyrY(i) = data( (i-1)*NSensors + 5 );
              gyrZ(i) = data( (i-1)*NSensors + 6 );
            end

 	    outvec = PerformCalibration(accX, accY, accZ, gyrX, gyrY, gyrZ);

	    block.outptr(1) = outvec;

	  case 4 // init
	    printf("init\n");

	  case 5 // terminate
	    printf("terminate\n");

	  case 10 // configure
	    printf("configure\n");
	    block.invecsize = NcalibSamples*NSensors;
	    block.outvecsize = 20;

	end
      endfunction
      

      // get the stored sensor data
      [sim, readI] = ld_const(sim, ev, 1); // start at index 1
      [sim, CombinedData] = ld_read_global_memory(sim, ev, index=readI, ident_str="IMU_EKF_SensorData", ...
                                                  datatype=ORTD.DATATYPE_FLOAT, ...
                                                  ElementsToRead=NcalibSamples*NSensors);

      // Embedded Scilab. Run the function scilab_comp_fn defined above for one time step to perform the calibration
      // that is implemented in Scilab.
      [sim, Calibration] = ld_scilab2(sim, 0, in=CombinedData, comp_fn=scilab_comp_fn, include_scilab_fns=list(), scilab_path="BUILDIN_PATH");

      // Print the results
      [sim] = ld_printf(sim, 0, Calibration, "The from Scilab returned values are ", 20);

      // Store the calibration into a shared memory
      [sim, one] = ld_const(sim, ev, 1);
      [sim] = ld_write_global_memory(sim, 0, data=Calibration, index=one, ...
                                     ident_str="IMU_EKF_CalibrationResult", datatype=ORTD.DATATYPE_FLOAT, ...
                                     ElementsToWrite=20);


      // Tell ld_AutoExperiment that the calibration was successful
      [sim, oneint32] = ld_constvecInt32(sim, 0, vec=1)
      CalibrationOk = oneint32;
  endfunction

  NSensors = 6;  NcalibSamples = 500;

  // initialise a global memory for storing the sensor-data for the calibration 
  [sim] = ld_global_memory(sim, ev, ident_str="IMU_EKF_SensorData", ... 
                           datatype=ORTD.DATATYPE_FLOAT, len=NcalibSamples*NSensors, ...
                           initial_data=[zeros(NcalibSamples*NSensors,1)], ... 
                           visibility='global', useMutex=1);

  // initialise a global memory for storing the calibration result
  [sim] = ld_global_memory(sim, ev, ident_str="IMU_EKF_CalibrationResult", ... 
                           datatype=ORTD.DATATYPE_FLOAT, len=20, ...
                           initial_data=[zeros(20,1)], ... 
                           visibility='global', useMutex=1);

  // Start the experiment
  ThreadPrioStruct.prio1=ORTD.ORTD_RT_NORMALTASK;
  ThreadPrioStruct.prio2=0, ThreadPrioStruct.cpu = -1;

  [sim, finished, outlist] = ld_AutoExperiment(sim, ev, inlist=list(AccGyro, Ts), insizes=[6,1], outsizes=[1], ...
                                     intypes=[ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT] , outtypes=[ORTD.DATATYPE_FLOAT], ...
                                     ThreadPrioStruct, experiment, whileComputing, evaluation, whileIdle);


  [sim] = ld_printf(sim, 0, finished, "State ", 1);

  q = 0; R = 0; g_s = 0;

endfunction






