// Modified from libuhd's tx_waveforms example.
// This is a two-tone generator for the B210 -- by M. H. Reilly kb1vc... 

// Original tx_waveforms: 
//
// Copyright 2010-2012,2014 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <uhd/utils/thread_priority.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/log.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/exception.hpp>
#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <iostream>


namespace po = boost::program_options;

/***********************************************************************
 * Main function
 **********************************************************************/
int main(int argc, char *argv[]){
    uhd::set_thread_priority_safe();

    //variables to be set by po
    std::string args, ant, subdev, ref, pps, channel_list;
    size_t spb;
    double rate, freq, sep, gain, bw;
    float ampl;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value(""), "single uhd device address args")
        ("freq", po::value<double>(&freq), "Lower RF frequency in Hz")
        ("sep", po::value<double>(&sep), "Separation between Lower and Upper RF frequency in Hz")
        ("ampl", po::value<float>(&ampl)->default_value(float(0.3)), "amplitude of the waveform [0 to 0.7]")
        ("gain", po::value<double>(&gain), "gain for the RF chain")
        ("ref", po::value<std::string>(&ref)->default_value("internal"), "clock reference (internal, external, mimo, gpsdo)")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("B210 Two Tone RF Generator") % desc << std::endl;
        return ~0;
    }

    //create a usrp device
    std::cout << std::endl;
    if(args.size() != 0) {
       args += std::string(", type=b200");
    }
    else args = std::string("type=b200");

    std::cout << boost::format("Creating the usrp device with: [%s]...") % args << std::endl;
    uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(args);




    //Lock mboard clocks
    usrp->set_clock_source(ref);

    uhd::usrp::subdev_spec_t twoports("A:A A:B");
    usrp->set_tx_subdev_spec(twoports);

    //create a transmit streamer
    //linearly map channels (index0 = channel0, index1 = channel1, ...)
    uhd::stream_args_t stream_args("fc32", "sc8");
    std::vector<size_t> channel_nums; 
    channel_nums.push_back(0);     channel_nums.push_back(1); 
    stream_args.channels = channel_nums;
    uhd::tx_streamer::sptr tx_stream = usrp->get_tx_stream(stream_args);

    
    std::cout << boost::format("Using Device: %s") % usrp->get_pp_string() << std::endl;

    rate = 625000;
    
    std::cout << boost::format("Setting TX Rate: %f Msps...") % (rate/1e6) << std::endl;
    usrp->set_tx_rate(rate);
    std::cout << boost::format("Actual TX Rate: %f Msps...") % (usrp->get_tx_rate()/1e6) << std::endl << std::endl;

    //set the center frequency
    if (not vm.count("freq")){
        std::cerr << "Please specify the center frequency with --freq" << std::endl;
        return ~0;
    }
    if (not vm.count("sep")){
        std::cerr << "Please specify the two-tone separation with --sep" << std::endl;
        return ~0;
    }

    // So, we're going to create TWO tune requests. The first is a proto, to find 
    // the middle freq setting. 
    uhd::tune_request_t tr(freq + 0.5 * sep); // this is the mid-freq setting. 
    uhd::tune_result_t tres; 
    tres = usrp->set_tx_freq(tr, 0);
    std::cout << boost::format("Actual MID TX Freq: %f MHz...\n") % (usrp->get_tx_freq(0) * 1e-6);
    std::cout << boost::format("\tRF_actual = %16.14g MHz  DUC = %g Hz\n")
      % (tres.actual_rf_freq * 1e-6)% tres.actual_dsp_freq;

    uhd::tune_request_t trs[2]; 
    trs[0] = uhd::tune_request_t(freq);
    trs[1] = uhd::tune_request_t(freq+sep);     

    trs[0].rf_freq = tres.actual_rf_freq; 
    trs[0].dsp_freq = freq - tres.actual_rf_freq;
    trs[0].rf_freq_policy = uhd::tune_request_t::POLICY_NONE;
    trs[0].dsp_freq_policy = uhd::tune_request_t::POLICY_MANUAL;

    trs[1].rf_freq = tres.actual_rf_freq; 
    trs[1].dsp_freq = (freq + sep) - tres.actual_rf_freq;
    trs[1].rf_freq_policy = uhd::tune_request_t::POLICY_NONE;
    trs[1].dsp_freq_policy = uhd::tune_request_t::POLICY_MANUAL;

    int i; 
    for(i = 0; i < 2; i++) {
      UHD_LOG << boost::format("!!!!Setting TX freq [%d] to rf=%16.14g dsp=%16.14g\n")
	% i % trs[i].rf_freq % trs[i].dsp_freq; 
      tres = usrp->set_tx_freq(trs[i], i);
      std::cout << boost::format("\nChan[%d] Actual TX Freq: %f MHz...\n") % i % (usrp->get_tx_freq(i) * 1e-6);
      std::cout << boost::format("\tRF_actual = %16.14g MHz  DUC = %g Hz target = %16.14g MHz\n")
	% (tres.actual_rf_freq * 1e-6)% tres.actual_dsp_freq % (usrp->get_tx_freq(i) * 1e-6);

      UHD_LOG << boost::format("\nChan[%d] Actual TX Freq: %f MHz...\n") % i % (usrp->get_tx_freq(i) * 1e-6);
      UHD_LOG << boost::format("\tRF_actual = %16.14g MHz  DUC = %g Hz target = %16.14g MHz\n")
	% (tres.actual_rf_freq * 1e-6)% tres.actual_dsp_freq % (usrp->get_tx_freq(i) * 1e-6);
    }
    
    for(i = 0; i < 2; i++) {
      double ff = usrp->get_tx_freq(i); 
      std::cout << boost::format("Check -- channel %d freq = %16.4g MHz\n") % i % (ff * 1e-6);
      UHD_LOG << boost::format("Check -- channel %d freq = %16.4g MHz\n") % i % (ff * 1e-6);      
    }
    

    boost::this_thread::sleep(boost::posix_time::seconds(1)); //allow for some setup time


    //set the rf gain
    if (vm.count("gain")){
      std::cout << boost::format("Setting TX Gain: %f dB...") % gain << std::endl;
      usrp->set_tx_gain(gain, 0);
      usrp->set_tx_gain(gain, 1);      
      std::cout << boost::format("Actual TX Gain: %f dB...") % usrp->get_tx_gain(0) << std::endl << std::endl;
    }

    
    //allocate a buffer which we re-use for each channel
    spb = tx_stream->get_max_num_samps()*10;
    std::vector<std::complex<float> > buff(spb);
    std::vector<std::complex<float> *> buffs(2, &buff.front());

    std::cout << boost::format("Setting device timestamp to 0...") << std::endl;

    //Check Ref and LO Lock detect
    std::vector<std::string> sensor_names;
    sensor_names = usrp->get_tx_sensor_names(0);
    if (std::find(sensor_names.begin(), sensor_names.end(), "lo_locked") != sensor_names.end()) {
        uhd::sensor_value_t lo_locked = usrp->get_tx_sensor("lo_locked",0);
        std::cout << boost::format("Checking TX: %s ...") % lo_locked.to_pp_string() << std::endl;
        UHD_ASSERT_THROW(lo_locked.to_bool());
    }
    sensor_names = usrp->get_mboard_sensor_names(0);

    if ((ref == "external") and (std::find(sensor_names.begin(), sensor_names.end(), "ref_locked") != sensor_names.end())) {
        uhd::sensor_value_t ref_locked = usrp->get_mboard_sensor("ref_locked",0);
        std::cout << boost::format("Checking TX: %s ...") % ref_locked.to_pp_string() << std::endl;
        UHD_ASSERT_THROW(ref_locked.to_bool());
    }

    // Set up metadata. We start streaming a bit in the future
    uhd::tx_metadata_t md;
    md.start_of_burst = true;
    md.end_of_burst   = false;
    md.has_time_spec  = true;
    md.time_spec = usrp->get_time_now() + uhd::time_spec_t(0.5);
    
    //fill the buffer with a constant waveform.
    for (size_t n = 0; n < buff.size(); n++){
      buff[n] = std::complex<float>(ampl,ampl);
    }
    std::cerr << "*****" << std::endl; 
    while(1) {
      tx_stream->send(buffs, buff.size(), md);
      md.start_of_burst = false;
      md.has_time_spec = false;
    }

    //send a mini EOB packet
    md.end_of_burst = true;
    tx_stream->send("", 0, md);

    //finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;
    return EXIT_SUCCESS;
}
