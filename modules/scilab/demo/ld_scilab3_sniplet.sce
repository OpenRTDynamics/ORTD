

      // define a Scilab function that performs the calibration
      function [block]=scilab_comp_fn( block, flag )
	// This scilab function is called during run-time
	// NOTE: Please note that the variables defined outside this
	//       function are typically not available at run-time.
	//       This also holds true for self defined Scilab functions!


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

	  printf("Calibrating Sensor with Sensorid " + string(Sensorid) + " ...\n");

          means=[ mean(accX); mean(accY); mean(accZ); mean(gyrX);  mean(gyrY);  mean(gyrZ) ];
          CalibOk = 1;

 	  printf("The means are: [ mean(accX); mean(accY); mean(accZ); mean(gyrX);  mean(gyrY);  mean(gyrZ) ]\n"); 
          disp(means);

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
              accX(i) = data( (i-1)*NSensorsPerUnit + 1 );
              accY(i) = data( (i-1)*NSensorsPerUnit + 2 );
              accZ(i) = data( (i-1)*NSensorsPerUnit + 3 );

              gyrX(i) = data( (i-1)*NSensorsPerUnit + 4 );
              gyrY(i) = data( (i-1)*NSensorsPerUnit + 5 );
              gyrZ(i) = data( (i-1)*NSensorsPerUnit + 6 );
            end

 	    outvec = PerformCalibration(accX, accY, accZ, gyrX, gyrY, gyrZ);

            // save the calibration results
            save("ld_IMU_OriEst_CalibrationData_Sensorid_" + string(Sensorid) + ".dat", data, accX, accY, accZ, gyrX, gyrY, gyrZ, outvec );  // FIXME: comment this

	    block.outptr(1) = outvec;

	  case 4 // init
	    printf("init\n");

	  case 5 // terminate
	    printf("terminate\n");

	  case 10 // configure I/O
	    printf("configure\n");
	    block.invecsize = NcalibSamples*NSensorsPerUnit;
	    block.outvecsize = 20;

	end
      endfunction
      

      // get the stored sensor data
      [sim, readI] = ld_const(sim, ev, 1); // start at index 1
      [sim, CombinedData] = ld_read_global_memory(sim, ev, index=readI, ident_str="IMU_EKF_SensorData_"+idStr, ...
                                                  datatype=ORTD.DATATYPE_FLOAT, ...
                                                  ElementsToRead=NcalibSamples*NSensorsPerUnit);

      // Embedded Scilab. Run the function scilab_comp_fn defined above for one time step to perform the calibration
      // that is implemented in Scilab. The variables NSensorsPerUnit and NcalibSamples will be automagically
      // transfered and defined in the on-line Scilab instance.
      NSensorsPerUnit = 6;  NcalibSamples = 500;

      [sim, Calibration] = ld_scilab3(sim, 0, in=CombinedData, comp_fn=scilab_comp_fn, include_scilab_fns=list(), ...
                                      InitStr="", scilab_path="BUILDIN_PATH");


      // Print the results
      [sim] = ld_printf(sim, 0, Calibration, "The from Scilab returned values are ", 20);

