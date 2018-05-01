# Encryption and Decryption

## Usage

### Getting started

- Run `./start.sh`, when using this for the first time. 

```
    # remove dev files if already present
    sudo rm /dev/decdev
    sudo rm /dev/encdev

    # makes device file
    sudo mknod /dev/decdev c 601 0
    sudo mknod /dev/encdev c 600 0

    # transfer access to the user
    sudo chown $USER /dev/decdev
    sudo chown $USER /dev/encdev

    # gives read and write access
    sudo chmod +rw /dev/decdev
    sudo chmod +rw /dev/encdev
```

- Now do `make`. It automatically inserts the module.   

- To remove the modules, do `make clean`.   

### Using the program

#### Hello World!

```
    echo "Hello World!" > /dev/encdev; cat /dev/encdev > /dev/decdev; cat /dev/decdev
```

#### In general
 
- `test.txt`: contains the input file. Must be under 320 characters. 
- `encrypted`: contains the encrypted form the the input file. 
- `decrypted_file.txt`: contains the decrypted form of the file `encrypted`  

- Encryption:  

    `cat test.txt > /dev/encdev; cat /dev/encdev > encrypted`

- Decryption:  

    `cat encrypted > /dev/decdev; cat /dev/decdev > decrypted_file.txt`


## Errors

- More than one user cannot access a single device simultaneously - EBUSY

- Overflowing buffer, ie, keep writing without even reading - Permission Denied and KERN_WARNING

- Reading without writing anything to the device - KERN_WARNING


## Implemenation

- Key is shared within the two kernel modules
- Random key is generated on the first write on loading or after EOF in the input by the encdev
- encdev should be loaded before decdev, while loading
- decdev should  unloaded before encdev, while unloading
- One can encrypt and decrypt stuff in batch of at max 320 characters
