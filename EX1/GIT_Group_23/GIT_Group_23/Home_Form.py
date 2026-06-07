import tkinter as tk
from tkinter import messagebox
try:
    from PIL import Image, ImageTk
except ImportError:
    print()
from Convert_Dec_2_Hex import Convert_Dec_2_Hex
from Convert_Hex_2_Dec import Convert_Hex_2_Dec

#global variables
First_Click = False
Output_Num_TextBox = None
label_result = None
input_val = None
Input_Num_TextBox = None

def Define_Output_Label_and_TextBox():
        global First_Click, Output_Num_TextBox, label_result, input_val

        #Output number text
        label_result = tk.Label(Main_Form, text="Invalid Input", font=("Arial", 10, "bold"), 
                        fg="black", bg=COLOR_BG)
        label_result.pack(pady=5)
        #TextBox to show the output number
        Output_Num_TextBox = tk.Entry(Main_Form, width=30, font=("Arial", 12), 
                                        bg="#2e2e2e", fg="white", 
                                        insertbackground="white", justify="center")
        Output_Num_TextBox.pack(pady=5)
        Output_Num_TextBox.config(state="readonly")

        return True

def Update_Output_TextBox(new_value):
    global Output_Num_TextBox
    Output_Num_TextBox.config(state="normal")      #open the textBox to changes
    Output_Num_TextBox.delete(0, tk.END)           #clear past data
    Output_Num_TextBox.insert(0, f"{new_value}")   #write new data
    Output_Num_TextBox.config(state="disabled")    #lock - back to read only


def handle_checkbox_click(clicked_type):    #CheckBoxes cant be toggled on together
    if clicked_type == "DEC":
        if Dec_2_Hex_CheckBox_Value.get() == 1:
            Hex_2_Dec_CheckBox_Value.set(0)
    
    elif clicked_type == "HEX":
        if Hex_2_Dec_CheckBox_Value.get() == 1:
            Dec_2_Hex_CheckBox_Value.set(0)


def on_convert_click():                     #event of Convert_Button-click
    global First_Click, Output_Num_TextBox, label_result, input_val, Input_Num_TextBox
    input_val = Input_Num_TextBox.get()     #save the input value

    if not input_val:                       #input = null/empty
        messagebox.showwarning("Input Error", "Please enter a number first!")
        return

    Input_Num_TextBox.delete(0, tk.END)     #clear the input textbox after clicking the button
    if Dec_2_Hex_CheckBox_Value.get() == 1: #if Dec_2_Hex checkbox is selected
        Bool_Val, ans = Convert_Dec_2_Hex(input_val)

        if (Bool_Val == False): #if the conversion failed
            messagebox.showerror("Conversion Error", "Invalid input for decimal to hexadecimal conversion.")
            return
        
    elif Hex_2_Dec_CheckBox_Value.get() == 1:
        Bool_Val, ans = Convert_Hex_2_Dec(input_val)

        if (Bool_Val == False): #if the conversion failed
            messagebox.showerror("Conversion Error", "Invalid input for hexadecimal to decimal conversion.")
            return
        
    else:                                   #if no checkbox is selected
        messagebox.showinfo("Selection Required", "Please select a conversion type")
    
    if not First_Click and Bool_Val:                     #if it's the first click and the input is valid - 
        First_Click = Define_Output_Label_and_TextBox()     #create the output textbox and "Your result is" label

    Update_Output_TextBox(ans)   #update the output textbox with the new answer
    label_result.config(text=f"Your Result for {input_val} is:")


def toggle_dec_hex(event):                  #if Dec_2_Hex_CheckBox toggled
    Dec_2_Hex_CheckBox.toggle()
    handle_checkbox_click("DEC")            #if toggled on - toggle off the other checkbox

def toggle_hex_dec(event): 
    Hex_2_Dec_CheckBox.toggle()
    handle_checkbox_click("HEX")

Main_Form = tk.Tk()                         #Define the main form
Main_Form.title("Numeric Converter (Hex / Dec) 2026")
Main_Form.geometry("720x480")

try:
    bg_image = Image.open("BackGround_image.png").resize((720, 480))
    bg_photo = ImageTk.PhotoImage(bg_image)
    bg_label = tk.Label(Main_Form, image=bg_photo)
    bg_label.place(x=0, y=0, relwidth=1, relheight=1)
except:
   Main_Form.configure(bg="#fcfcfc") #in case there is a problem with the image's path

COLOR_BG = "#fcfcfc"                      #main color. used in the design

frame1 = tk.Frame(Main_Form, bg=COLOR_BG)   #text 1 - "Convert Dec to Hex"
frame1.pack(pady=(50, 10))

Dec_2_Hex_CheckBox_Value = tk.IntVar()      #define a CheckBox value

lbl1 = tk.Label(frame1, text="Convert Dec to Hex", font=("Arial", 10), #design of the text
                fg="black", bg=COLOR_BG, cursor="hand2")
lbl1.pack(side="left")
lbl1.bind("<Button-1>", toggle_dec_hex)

Dec_2_Hex_CheckBox = tk.Checkbutton(frame1, variable=Dec_2_Hex_CheckBox_Value, 
                              command=lambda: handle_checkbox_click("DEC"),
                              bg=COLOR_BG, selectcolor="white", 
                              activebackground=COLOR_BG, highlightthickness=0, 
                              bd=0, cursor="hand2")
Dec_2_Hex_CheckBox.pack(side="left", padx=5)

frame2 = tk.Frame(Main_Form, bg=COLOR_BG)   #text 2 - "Convert Hex to Dec"
frame2.pack(pady=10)
Hex_2_Dec_CheckBox_Value = tk.IntVar()

lbl2 = tk.Label(frame2, text="Convert Hex to Dec", font=("Arial", 10), 
                fg="black", bg=COLOR_BG, cursor="hand2")
lbl2.pack(side="left")
lbl2.bind("<Button-1>", toggle_hex_dec)

Hex_2_Dec_CheckBox = tk.Checkbutton(frame2, variable=Hex_2_Dec_CheckBox_Value, 
                              command=lambda: handle_checkbox_click("HEX"),
                              bg=COLOR_BG, selectcolor="white", 
                              activebackground=COLOR_BG, highlightthickness=0, 
                              bd=0, cursor="hand2")
Hex_2_Dec_CheckBox.pack(side="left", padx=5)

#"Enter your number" text
label_instr = tk.Label(Main_Form, text="Enter your number:", font=("Arial", 10, "bold"), 
                       fg="black", bg=COLOR_BG)
label_instr.pack(pady=(30, 5))
Input_Num_TextBox = tk.Entry(Main_Form, width=30, font=("Arial", 12), 
                             bg="#2e2e2e", fg="white", 
                             insertbackground="white", justify="center")
Input_Num_TextBox.pack(pady=5)

#define the button
Convert_Button = tk.Button(Main_Form, text="Convert Now", command=on_convert_click, 
                           bg="#BDD7EE", font=("Arial", 10, "bold"), 
                           width=20, height=2)
Convert_Button.pack(pady=(20,80))

Main_Form.bind('<Return>', lambda event: on_convert_click())
Main_Form.mainloop()





