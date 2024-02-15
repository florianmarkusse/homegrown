# Folder with bad ideas

run 
```
./install-dependencies.sh
echo export PATH=\$PATH:~/opt/cross/bin >> ~/.bashrc # or whatever shell u use.  
```

Will currently (probably) break intermittently because of interrupts clobbering the red zone. Set up separate stack to handle interrupts ok.


