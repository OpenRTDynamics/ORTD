function [sim, finished, outlist] = ld_AutoExperiment(sim, ev, inlist, insizes, outsizes, intypes, outtypes, ThreadPrioStruct, experiment_fn, whileComputing_fn, evaluation_fn, whileIdle_fn)  // PARSEDOCU_BLOCK
    //
    // %PURPOSE: Perform automatic calibration procedures
    // 
    // Automatically perform an experiment and the ongoing evaluation.
    // The computation required for the evaluation is performed in a the background
    // by means of a thread.
    // 
    // There are several callback functions that describe:
    // 
    // experiment_fn: The schematic for performing the experiment, e.g. collecting data
    // whileComputing_fn: The schematic that is activated during the computation is active_state
    // evaluation_fn: The schematic that performs the evaulation in a thread.
    //                One simulation step is performed here
    // whileIdle_fn:  The schematic that is active when the procedure finished.
    // 
    // 
    // 
    // 
    // The prototypes are (Hereby outlist and inlist are lists of the signals that are forwarded 
    // to the I/O of ld_AutoExperiment:
    // 
    // [sim, finished, outlist] = experiment_fn(sim, ev, inlist)
    // [sim, outlist] = whileComputing_fn(sim, ev, inlist)
    // [sim, CalibrationOk, userdata] = evaluation_fn(sim, userdata)
    // [sim, outlist] = whileIdle_fn(sim, ev, inlist)
    // 
    // NOTE: Not everything is finished by now
    // 


    function [sim, outlist, userdata] = evaluation_Thread(sim, inlist, userdata)

        [sim, CalibrationOk, userdata] = evaluation_fn(sim, userdata);

        outlist = list(CalibrationOk);
    endfunction

    function [sim, outlist, active_state, x_global_kp1, userdata] = experiment_sm(sim, inlist, x_global, state, statename, userdata)
        // This function is called multiple times: once for each state.
        // At runtime these are different nested simulations. Switching
        // between them is done, where each simulation represents a
        // certain state.

        ev = 0;
        printf("ld_AutoExperiment: defining state %s (#%d) ...\n", statename, state);


        // print out some state information
        //     [sim] = ld_printf(sim, ev, in=x_global, str="<cntrl_state "+statename+"> x_global", insize=1);


        // define different controllers here
        select state
        case 1 // state 1
            // The experiment
            [sim, finished, outlist, userdata] = experiment_fn(sim, ev, inlist, userdata);

            [sim, active_state] = ld_const(sim, ev, 0);  // by default: no state switch       
            [sim, active_state ] = ld_cond_overwrite(sim, ev, in=active_state, condition=finished, setto=2); // Go to state 2 when finished

        case 2 // state 2
            // run something while the computation is running
            [sim, outlist, userdata] = whileComputing_fn(sim, ev, inlist, userdata);

            // Create a thread for performing the computation in the background
            [sim, startcalc] = ld_initimpuls(sim, 0); // triggers the computation only once when entering this state
            [sim, outlist__, computation_finished] = ld_async_simulation(sim, 0, ...
            inlist=list(), ...
            insizes=[], outsizes=[1], ...
            intypes=[], outtypes=[ORTD.DATATYPE_INT32], ...
            nested_fn = evaluation_Thread, ...
            TriggerSignal=startcalc, name="Comp Thread", ...
            ThreadPrioStruct, userdata=list() );


            //
            CalibrationOk = outlist__(1);
            [sim,CalibrationOk_] = ld_Int32ToFloat(sim, 0, CalibrationOk);

            // 	  [sim] = ld_printf(sim, ev, in=computation_finished, str="computation_finished", insize=1);

            [sim, FinshedOk] = ld_and(sim, 0, list( CalibrationOk_, computation_finished ));

            // WHEN TO CHANGE THE STATE
            [sim, active_state] = ld_const(sim, ev, 0);  // by default: no state switch
            [sim, active_state ] = ld_cond_overwrite(sim, ev, in=active_state, condition=computation_finished, setto=3); // go to state 3 if        

        case 3 // state 3
            [sim, outlist, userdata] = whileIdle_fn(sim, ev, inlist, userdata);

            [sim, active_state] = ld_const(sim, ev, 0);  // by default: no state switch       
        end

        x_global_kp1 = x_global;    
    endfunction




    // set-up three states represented by three nested simulations
    [sim, outlist, x_global, active_state,userdata] = ld_statemachine(sim, ev=ev, ...
    inlist, ..
    insizes, outsizes, ... 
    intypes, outtypes, ...
    nested_fn=experiment_sm, Nstates=3, state_names_list=list("experiment", "evaluation", "finished"), ...
    inittial_state=1, x0_global=[1], userdata=list()  );


    finished = active_state;
endfunction



function [sim, finished, outlist, userdata] = ld_AutoExperiment2(sim, ev, inlist, insizes, outsizes, intypes, outtypes, ThreadPrioStruct, experiment_fn, whileComputing_fn, evaluation_fn, whileIdle_fn, userdata)  // PARSEDOCU_BLOCK
    //
    // Automatically perform an experiment and the ongoing evaluation.
    // The computation required for the evaluation is performed in a the background
    // by means of a thread.
    // 
    // There are several callback functions that describe:
    // 
    // experiment_fn: The schematic for performing the experiment, e.g. collecting data
    // whileComputing_fn: The schematic that is activated during the computation is active_state
    // evaluation_fn: The schematic that performs the evaulation in a thread.
    //                One simulation step is performed here
    // whileIdle_fn:  The schematic that is active when the procedure finished.
    // 
    // 
    // 
    // 
    // The prototypes are (Hereby outlist and inlist are lists of the signals that are forwarded 
    // to the I/O of ld_AutoExperiment:
    // 
    // [sim, finished, outlist] = experiment_fn(sim, ev, inlist)
    // [sim, outlist] = whileComputing_fn(sim, ev, inlist)
    // [sim, CalibrationReturnVal, userdata] = evaluation_fn(sim, userdata)  NOTE changed this function
    // [sim, outlist] = whileIdle_fn(sim, ev, inlist)
    // 
    // NOTE: Not everything is finished by now
    // 
    // Rev 2: of ld_AutoExperiment: added userdata input/output, changed prototype of evaluation_fn
    // 


    function [sim, outlist, userdata] = evaluation_Thread(sim, inlist, userdata)
        [sim, CalibrationReturnVal, userdata] = evaluation_fn(sim, userdata);

        outlist = list(CalibrationReturnVal);
    endfunction

    function [sim, outlist, active_state, x_global_kp1, userdata] = experiment_sm(sim, inlist, x_global, state, statename, userdata)
        // This function is called multiple times: once for each state.
        // At runtime these are different nested simulations. Switching
        // between them is done, where each simulation represents a
        // certain state.



        ev = 0;
        printf("ld_AutoExperiment: defining state %s (#%d) ...\n", statename, state);


        // print out some state information
        //     [sim] = ld_printf(sim, ev, in=x_global, str="<cntrl_state "+statename+"> x_global", insize=1);


        // define different controllers here
        select state
        case 1 // state 1
            // The experiment

            [sim, finished, outlist, userdata] = experiment_fn(sim, ev, inlist, userdata);

            [sim, active_state] = ld_const(sim, ev, 0);  // by default: no state switch       
            [sim, active_state ] = ld_cond_overwrite(sim, ev, in=active_state, condition=finished, setto=2); // Go to state 2 when finished

        case 2 // state 2


            // Create a thread for performing the computation in the background
            [sim, startcalc] = ld_initimpuls(sim, 0); // triggers the computation only once when entering this state
            [sim, outlist__, computation_finished, userdata] = ld_async_simulation(sim, 0, ...
            inlist=list(), ...
            insizes=[], outsizes=[1], ...
            intypes=[], outtypes=[ORTD.DATATYPE_INT32], ...
            nested_fn = evaluation_Thread, ...
            TriggerSignal=startcalc, name="Comp Thread", ...
            ThreadPrioStruct, userdata );


            //
            CalibrationReturnVal = outlist__(1);
            [sim, CalibrationReturnVal_] = ld_Int32ToFloat(sim, 0, CalibrationReturnVal);
            [sim, JumperVals_] = ld_jumper(sim, 0, in=CalibrationReturnVal_, steps=2);
            [sim, JumperVals ] = ld_demux(sim, 0, 2, JumperVals_);

            //
            [sim, NotFinished] = ld_not(sim, 0, computation_finished);

            //           [sim, NotCalibrationOk_ ] = ld_not(sim, 0, CalibrationOk_);
            // 
            // 	  [sim] = ld_printf(sim, ev, in=computation_finished, str="computation_finished", insize=1);
            //          [sim] = ld_printf(sim, ev, in=NotFinished, str="computation_Notfinished", insize=1);// 

            //           [sim, FinshedOk   ] = ld_and(sim, 0, list(    CalibrationOk_ , computation_finished ));
            //           [sim, FinshedButNotOk] = ld_and(sim, 0, list( NotCalibrationOk_ , computation_finished ));


            // run something while the computation is running
            [sim, outlist, HoldState, userdata] = whileComputing_fn(sim, ev, inlist, CalibrationReturnVal, computation_finished, userdata);


            // WHEN TO CHANGE THE STATE
            [sim, active_state] = ld_const(sim, ev, 0);  // by default: no state switch
            [sim, active_state ] = ld_cond_overwrite(sim, ev, in=active_state, condition=JumperVals(1)  , setto=1); // return val 0 --> redo the calibration
            [sim, active_state ] = ld_cond_overwrite(sim, ev, in=active_state, condition=JumperVals(2)  , setto=3); // return val 1 --> run state 3

            [sim, active_state ] = ld_cond_overwrite(sim, ev, in=active_state, condition=NotFinished, setto=0); // If the computation is not finsihed do nothing at all
            [sim, active_state ] = ld_cond_overwrite(sim, ev, in=active_state, condition=HoldState, setto=0); // If whileComputing_fn desires to hold this state do nothing at all







        case 3 // state 3
            [sim, outlist, userdata] = whileIdle_fn(sim, ev, inlist, userdata);

            [sim, active_state] = ld_const(sim, ev, 0);  // by default: no state switch       
        end

        x_global_kp1 = x_global;    
    endfunction



    //  pause;
    // set-up three states represented by three nested simulations
    [sim, outlist, x_global, active_state,userdata] = ld_statemachine(sim, ev=ev, ...
    inlist, ..
    insizes, outsizes, ... 
    intypes, outtypes, ...
    nested_fn=experiment_sm, Nstates=3, state_names_list=list("experiment", "evaluation", "finished"), ...
    inittial_state=1, x0_global=[1], userdata  );


    finished = active_state;
endfunction




function [sim, finished, outlist, userdata] = ld_AutoOnlineExch_dev(sim, ev, inlist, insizes, outsizes, intypes, outtypes, ThreadPrioStruct, CallbackFns, ident_str, userdata)  // PARSEDOCU_BLOCK
    // 
    // %PURPOSE: Automated definition/compilation and execution of ORTD-schemtics during runtime.
    // 
    // Automatically perform an experiment and an ongoing evaluation. Additionally,
    // during this evaluation a new ORTD-schematic may be compiled to replace a part
    // of the control system.
    // 
    // The signals in inlist will be forwarded to several callback functions for defining e.g.
    // the nested control system.
    // 
    // There must be several callback functions in a structure CallbackFns:
    // 
    // experiment:      The schematic for performing the experiment, e.g. collecting data. This function
    //                  to define such a schematic may be called during compilation as well as during 
    //                  runtime of the control system. The latter case is used to replace the experiment
    //                  controller with an online-generated replacement that may depend e.g. on previously 
    //                  collected calibration data.
    // whileComputing:  The schematic that is activated during the computation is active_state
    // PreScilabRun:    This ORTD-schematic is called for one time step in advance to the embedded Scilab-calculation
    // 
    // 
    // Note: This is a temporary development version. The interface may slightly change. Rev 1
    // 


    // extract the callback functions
    experiment_user = CallbackFns.experiment;
    whileComputing_user = CallbackFns.whileComputing;
    PreScilabRun_user = CallbackFns.PreScilabRun;

    function [sim, outlist, userdata ] = ExperimentReplaceable(sim, inlist, par)
        //    
        //    The nested simulation contains two sub-simulations:
        //    
        //    1) A schematic, which commonly contains nothing and is switched to
        //       when the replacement is in progress (which may take some time)
        //
        //    2) The schematic, which actually contains the algorithm to execute
        //
        //    Here, the initial simulations are defined, which can then be 
        //    replaced during runtime
        //    


        ev = 0;

        cntrlN = par(1); // the number of the nested schematics (one of two) "1" means the 
        // dummy schematic which is activated while the 2nd "2" is exchanged during runtime
        userdata = par(2);

        printf("Compiling replaceable N=%d\n", cntrlN);   

        insizes = userdata(1); intypes = userdata(2); 
        outsizes=userdata(3); outtypes=userdata(4);
        useruserdata = userdata(5);
        experiment_user = userdata(6);

        [sim,zero] = ld_const(sim, ev, 0);

        if (cntrlN == 1) then  // is this the schematic 2) ?
            // Define dummy outputs
            // NOTE: only ORTD.DATATYPE_FLOAT is supported by this
            outlist = list(); 
            for outsize=outsizes        
                [sim, dummyOut] = ld_constvec(sim, 0, zeros( outsize, 1 ) );
                outlist($+1) = dummyOut;

            end
            outlist($+1) = zero; // control output
        end

        if (cntrlN == 2) then  // is this the schematic 2) ?

            printf("Compiling schematic: experiment_user\n");

            // run the callback function for definition of the experiment controller
            //       disp(fun2string(experiment_user));
            [sim, finished, outlist, useruserdata] = experiment_user(sim, 0, inlist, useruserdata, CalledOnline);

            printf("Compiling schematic: done\n");

            // store the userdata of experiment_user
            userdata(5) = useruserdata;

            // add control output
            outlist($+1) = finished;
        end

    endfunction


    function [sim, finished, outlist, userdata] = experiment(sim, ev, inlist, userdata)
        // Do the experiment

        insizes = userdata(1); intypes = userdata(2); 
        outsizes=userdata(3); outtypes=userdata(4);
        useruserdata = userdata(5);
        experiment_user = userdata(6);
        ident_str = userdata(7);

        [sim,zero] = ld_const(sim, ev, 0);
        [sim,active_sim] = ld_const(sim, 0, 1);

        //
        // Here the experiment controller is nested such that it can be replaced online
        //

        // append control signal to the outputs
        outsizes__ = outsizes; outsizes__($+1) = 1;
        outtypes__ = outtypes; outtypes__($+1) = ORTD.DATATYPE_FLOAT;


        CalledOnline = %f;  // The experiment function is not called online when going through 
        dfeed = 1;
        [sim, outlist_42342, computation_finished, userdata] = ld_simnest2(sim, 0 , ...
        inlist, ...
        insizes, outsizes__, ...
        intypes, ...
        outtypes__, ...
        nested_fn=ExperimentReplaceable, Nsimulations=2, dfeed, ...
        asynchron_simsteps=0, switch_signal=active_sim, ...
        reset_trigger_signal=zero, userdata, ...
        ident_str+"_ReplaceableSimulation" );

        finished = outlist_42342($); // the additional output

        //       [sim] = ld_printf(sim, 0, finished, "experiment: finished? " , 1);


        outlist = list( outlist_42342(1:$-1) );  // cut the last entry
    endfunction

    function [sim, outlist, HoldState, userdata] = whileComputing(sim, ev, inlist, CalibrationReturnVal, computation_finished, userdata);
        //   function [sim, outlist, userdata] = whileComputing(sim, ev, inlist, userdata)
        insizes = userdata(1); intypes = userdata(2); 
        outsizes=userdata(3); outtypes=userdata(4);
        useruserdata = userdata(5);
        experiment_user = userdata(6);

        // Callback
        par.userdata = useruserdata;
        par.insizes = insizes;
        par.outsizes = outsizes;
        par.intypes = intypes;
        par.outtypes = outtypes;

        [sim, outlist, HoldState, userdata] = whileComputing_user(sim, ev, inlist, CalibrationReturnVal, computation_finished, par);

    endfunction

    function [sim, outlist, userdata] = whileIdle(sim, ev, inlist, userdata)

        insizes = userdata(1); intypes = userdata(2); 
        outsizes=userdata(3); outtypes=userdata(4);
        useruserdata = userdata(5);
        experiment_user = userdata(6);
        ident_str = userdata(7);
        //       // Do wait
        //       [sim, readI] = ld_const(sim, ev, 1); // start at index 1
        //       [sim, Calibration] = ld_read_global_memory(sim, ev, index=readI, ident_str="CalibrationResult", ...
        //                                                   datatype=ORTD.DATATYPE_FLOAT, ...
        //                                                   ElementsToRead=20);

        //       [sim] = ld_printf(sim, 0, Calibration, "The calibration result is ", 20);

        //       // TODO: Dummy output      
        //       [sim, out] = ld_const(sim, ev, 0);
        //       outlist=list(out);

        // Define dummy outputs
        // NOTE: only ORTD.DATATYPE_FLOAT is supported by this
        outlist = list(); 
        for outsize=outsizes        
            [sim, dummyOut] = ld_constvec(sim, 0, zeros( outsize, 1 ) );
            outlist($+1) = dummyOut;
        end
    endfunction

    function [sim, CalibrationReturnVal, userdata] = evaluation(sim, userdata)
        // This superblock will run the evaluation of the experiment in a thread.
        // The superblock describes a sub-simulation, whereby only one step is simulated
        // which is enough to call scilab one signle time

        ev = 0;

        insizes = userdata(1); intypes = userdata(2); 
        outsizes=userdata(3); outtypes=userdata(4);
        useruserdata = userdata(5);
        experiment_user = userdata(6);
        ident_str = userdata(7);


        // define a Scilab function that performs the calibration
        function [block]=scilab_comp_fn( block, flag )
            // This scilab function is called during run-time
            // NOTE: Please note that the variables defined outside this
            //       function are typically not available at run-time.
            //       This also holds true for self defined Scilab functions!

            select flag
            case 1 // output
                tic();

                // split the sensor data
                data = block.inptr(1);

                block.states.ExecCounter = block.states.ExecCounter + 1;

                // get userdata
                try 
                    userdata = block.states.userdata;
                catch
                    userdata.isInitialised = %f;
                end

                // userdata should contain the sensor data
                userdata.SchematicInfo = "This schemtic was compiled during runtime in iteration #" + string(block.states.ExecCounter);
                userdata.InputData = data;


                printf("Parameters to this computational Scilab function:\n");
                disp(cfpar);


                insizes = cfpar.insizes;
                outsizes = cfpar.outsizes;
                intypes = cfpar.intypes;
                outtypes = cfpar.outtypes;
                ident_str = cfpar.ident_str;


                // 	    insizes=[2]; outsizes=[1];
                // 	    intypes=[ORTD.DATATYPE_FLOAT]; outtypes=[ORTD.DATATYPE_FLOAT];
                //             ident_str = "AutoCalibDemo";

                // append control signal to the outputs
                outsizes__ = outsizes; outsizes__($+1) = 1;
                outtypes__ = outtypes; outtypes__($+1) = ORTD.DATATYPE_FLOAT;

                //             printf("Defining schematic: experiment_user using the following function:\n");
                //             disp(fun2string(experiment_user));

                printf("__ userdata is\n"); disp(userdata);

                // create a new ir-par Experiment.[i,r]par files
                CalledOnline = %t;  // The experiment function is called online because we are in embedded Scilab here
                N = 2;
                [par, userdata] = ld_simnest2_replacement( ...
                insizes, outsizes__, ...
                intypes, outtypes__, ...
                nested_fn=ExperimentReplaceable, ...
                userdata=list(insizes, intypes, outsizes, outtypes, userdata, experiment_user), N);

                block.states.userdata = userdata(5);

                //
                printf("__ New userdata is\n"); disp(block.states.userdata);

                // save vectors to a file
                save_irparam(par, ident_str+'_ReplaceableSimulation.ipar', ident_str+'_ReplaceableSimulation.rpar');

                // Tell that everything went fine.
                compready = 1;
                CalibrationReturnVal = 0; // run the experiment again

                // pack
                outvec = zeros(20,1);

                outvec(1) = compready;
                outvec(2) = CalibrationReturnVal;

                // clear
                par.ipar = [];
                par.rpar = [];

                block.outptr(1) = outvec;

                ElapsedTime = toc();
                printf("Time to run the embedded Scilab Code %f sec.\n", ElapsedTime);

            case 4 // init
                // printf("Setting funcproc(0), which has been of value %d before\n", funcprot());
                //funcproc(0);
                block.states.ExecCounter = 0;

            case 5 // terminate
                // 	    printf("terminate\n");

            case 10 // configure

            end
        endfunction


        par.userdata = useruserdata;

        // run callback
        [sim, ToScilab, useruserdata] = PreScilabRun_user(sim, ev, par);
        [ToScilab_Size, ToScilab_type] = ld_getSizesAndTypes(sim, 0, SignalList=list(ToScilab) );

        //
        // Embedded Scilab. Run the function scilab_comp_fn defined above for one time step to perform the calibration
        // that is implemented in Scilab.
        //

        par.include_scilab_fns = list(ExperimentReplaceable, "ExperimentReplaceable", experiment_user, "experiment_user");

        function str = vec2str(v)
            LF = char(10);  // line feed char
            str = "[";
            for i=1:(length(v)-1)
                str = str + string(v(i)) + "," + LF;
            end
            str = str + string( v($) ) + "];";
        endfunction

        LF = char(10);
        par.InitStr = "cfpar.insizes=" + vec2str(insizes) + LF + "cfpar.intypes=" + vec2str(intypes) + LF...
        + "cfpar.outsizes=" + vec2str(outsizes) + LF + "cfpar.outtypes=" + vec2str(outtypes) + LF ...
        + "cfpar.ident_str=''" + ident_str + "'' " + LF;

        //        par.InitStr = "";
        //        disp(par.InitStr);


        par.scilab_path = "BUILDIN_PATH";
        [sim, Calibration] = ld_scilab4(sim, 0, in=ToScilab, invecsize=ToScilab_Size(1), outvecsize=20, ...
        comp_fn=scilab_comp_fn, ForwardVars=%f, par);

        // Print the results
        // [sim] = ld_printf(sim, 0, Calibration, "The from Scilab returned values are ", 20);

        // demux      
        [sim, one] = ld_const(sim, 0, 1);      [sim, two] = ld_const(sim, 0, 2);
        [sim, compready] = ld_extract_element(sim, 0, invec=Calibration, pointer=one, vecsize=20 );
        [sim, CalibrationReturnVal] = ld_extract_element(sim, 0, invec=Calibration, pointer=two, vecsize=20 );


        // replace schematic the experiment schematic
        [sim, exchslot] = ld_const(sim, 0, 2);
        [sim, out] = ld_nested_exchffile(sim, 0, compresult=compready, slot=exchslot, ... 
        fname=ident_str+"_ReplaceableSimulation", ident_str+"_ReplaceableSimulation");

        //       [sim] = ld_FlagProbe(sim, 0, in=out, str="ASYNC COMP", 1);


        // run callback
        //       [sim, useruserdata] = PostScilabCalc(sim, 0, Calibration, useruserdata);


        //       // Store the calibration into a shared memory
        //       [sim, one] = ld_const(sim, ev, 1);
        //       [sim] = ld_write_global_memory(sim, 0, data=Calibration, index=one, ...
        //                                      ident_str="CalibrationResult", datatype=ORTD.DATATYPE_FLOAT, ...
        //                                      ElementsToWrite=20);

        // 
        userdata(5) = useruserdata;

        // Tell ld_AutoExperiment to run the experiment again, 
        //       [sim, CalibrationReturnVal] = ld_not(sim, 0, compready);

        //       [sim, oneint32] = ld_constvecInt32(sim, 0, vec=1)
        //       CalibrationReturnVal = oneint32;
    endfunction




    // Userdata for experiment_user
    //   userdata = list("This schemtic was compiled in advance to the execution");
    userdata = [];
    userdata.SchemeticInfo = "This schemtic was compiled in advance to the execution";


    [sim, finished, outlist, userdata] = ld_AutoExperiment2(sim, ev, inlist, ...
    insizes, outsizes, ...
    intypes , outtypes, ...
    ThreadPrioStruct, experiment, whileComputing, evaluation, whileIdle, ...
    userdata=list(insizes, intypes, outsizes, outtypes, userdata, experiment_user, ident_str)  );

endfunction




