import codecs

class PCISubDevice:
    def __init__(self, subvendor_id, subdevice_id, name):
        self.subvendor_id = subvendor_id
        self.subdevice_id = subdevice_id
        self.name = name

    def __str__(self):
        return f'{self.subvendor_id}:{self.subdevice_id} {self.name}'

    def __repr__(self):
        return f'{self.subvendor_id}:{self.subdevice_id} {self.name}'

class PCIDevice:
    def __init__(self,  device_id, name):
        self.device_id = device_id
        self.name = name
        self.subdevices=[]

    def __str__(self):
        return f'{self.device_id} {self.name}'

    def __repr__(self):
        return f'{self.device_id} {self.name}'
    
class PCIVendor:
    def __init__(self, vendor_id, name):
        self.vendor_id = vendor_id
        self.name = name
        self.devices = {}

    def __str__(self):
        return f'{self.vendor_id} {self.name}'

    def __repr__(self):
        return f'{self.vendor_id} {self.name}'



def parse_pci_ids(file_path):
    pcis = {}
    current_vendor = None
    current_device = None

    with codecs.open(file_path, 'r','utf-8') as file:
        for line in file:            

            # 忽略空行和注释行
            if not line or line.startswith('#') or line.startswith('\n'):
                continue

            if line[0]=='C':
                break

            # 处理供应商ID
            if line[0]!='\t':
                data = line.split(' ', 1)
                vendor_id = data[0]
                vendor_name = data[1].strip()
                assert(vendor_id not in pcis)
                vendor = PCIVendor (vendor_id, vendor_name)
                pcis[vendor_id] = vendor
                current_vendor = vendor
            # 处理设备ID
            else:
                if line[0]=='\t' and line[1]=='\t':
                    data = line.split(' ', 2)
                    subdev_vendor = data[0].strip()
                    subdev_id = data[1].strip()
                    subdev_name = data[2].strip()
                    assert(current_vendor is not None)
                    assert(current_device is not None)
                    subdevice = PCISubDevice(subdev_vendor, subdev_id, subdev_name)
                    current_device.subdevices.append(subdevice)             
                    
                elif line[0]=='\t':
                    data = line.split(' ', 1)
                    device_id = data[0].strip()
                    device_name = data[1].strip()
                    assert(current_vendor is not None)
                    device = PCIDevice(device_id, device_name)
                    assert(device_id not in current_vendor.devices)
                    current_vendor.devices[device_id] = device
                    current_device= device
    return pcis

def print_device(id,name,delim,file):
    if delim:
        print(',',file=file)
    print("{",end='',file=file)
    print(f'"{id}",',end='',file=file)
    if(name.find('"')!=-1):
        name=name.replace('"','\\"')
    print(f'u8"{name}"',end='',file=file)
    print("}",end='',file=file)
    

def print_pci(vendor,keys,file):
    print(vendor)
    for device_id, device in vendor.devices.items():
        match = False
        for key in keys:
            if key in device.name:
                match=True
                break
        if not match:
            continue
        print("{",end='',file=file)
        print(f'"0x{device_id}",',end='',file=file)
        print("{",end='',file=file)        
        print_device("",device.name,False,file)
        for subdevice in device.subdevices:
            subid = subdevice.subvendor_id+subdevice.subdevice_id
            print_device(subid,subdevice.name,True,file)
        print("}",end='',file=file)
        print("},",end='',file=file)
pci_ids_dict = parse_pci_ids('pci.ids')
with open('intel.data', 'w',encoding='utf-8') as fs:    
    print_pci(pci_ids_dict['8086'],['UHD','Iris','Graphics'],fs)
with open('nvidia.data', 'w',encoding='utf-8') as fs:   
    print_pci(pci_ids_dict['10de'],['GeForce','Quadro','Tesla','TITAN'],fs)
with open('amd.data', 'w',encoding='utf-8') as fs:
    print_pci(pci_ids_dict['1002'],['Radeon'],fs)
with open('vmware.data', 'w',encoding='utf-8') as fs:
    print_pci(pci_ids_dict['15ad'],['VGA'],fs)
with open('zhaoxin.data', 'w',encoding='utf-8') as fs:
    print_pci(pci_ids_dict['1d17'],['GPU'],fs)

