import h5py as h5


def PrepareSNIRF(raw_filepath:str, processed_filepath:str):
    
    
    # Check what metadata the processed file has, 
    raw = h5.File(raw_filepath, mode='r')
    # If it lacks soruces and detector position we add it. 
    processed = h5.File(processed_filepath, mode='a')   
    
    raw_probe = raw["nirs"]["probe"]
    processed_probe = processed["nirs"]["probe"]
    
    for key in raw_probe.keys():
        print(key)
        
    for key in processed_probe.keys():
        print(key)
        
    return


if __name__ == "__main__":
    raw_filepath = 'C:\\dev\\NIRSViz\\Assets\\NIRS\\raw_data.snirf'
    processed_filepath = 'C:\\dev\\NIRSViz\\Assets\\NIRS\\satori_processed.snirf'
    PrepareSNIRF(raw_filepath, processed_filepath)