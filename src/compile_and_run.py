import os
import subprocess 

# def compile_file(filename):
#     if os.system("g++ -std=c++11 -g "+filename+".cpp -pthread -lrt -o "+filename) == 0:
#         print (filename+".cpp Compiled")
#     else:
#         print ("cannot compile")


def run_file(filename):

    command="g++ -std=c++11 -g "+filename+".cpp -pthread -lrt -o "+filename
    
    # print(command)
    # subprocess.call(["g++", "-std=c++11", "-g", filename+".cpp", "-pthread", "-lrt", "-o",filename])
    # subprocess.call("./"+filename)
    print("========================Compiling File "+filename+".cpp========================")
    x = subprocess.getoutput(command)
    
    if x == "": 
        print("==========================Executing File "+filename+"==========================")                        # no error/warning messages
        subprocess.run('./'+filename)        # run the program
    else:
        print(x)                        # display the error/warning




def main():
    files=['proposed','proposed_groomed','traditional','traditional_groomed','toggling','toggling_groomed']
    
    
    while(1):
        print('\n============================MENU==============================')
        i=1
        for file in files:
            print("Press "+str(i)+" to run "+file+"....")
            i=i+1
        print("Press 0 to not run any file and exit....")
        print('============================MENU==============================\n')
        
    
        try:
            choice=int(input())
        except ValueError:
            continue

        print('\n')

        if(choice>=1 and choice<9):
            run_file(files[choice-1])
        else:
            break

if __name__ == "__main__":
    main()
