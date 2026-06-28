import tkinter as tk
from tkinter import filedialog, ttk, messagebox
import os
import sys
import json

# --- Settings Configuration ---

DEFAULT_FILTERS = {
    "mode": "Blacklist",
    "extensions": [
        ".pyc", ".log", ".tmp", ".DS_Store", ".ini", ".db", ".ico", ".png",
        ".jpg", ".resx", ".aps", ".txt"
    ],
    "ignore_files": [
        "vcxproj.filters",
        "vcxproj.user"
    ],
    "ignore_folders": [
        ".git", ".vs", "__pycache__", "node_modules", "obj", "bin",
        "Properties"
    ]
}

def get_script_directory():
    """ Get the directory of the script or the executable. """
    if getattr(sys, 'frozen', False):
        # If the application is run as a bundle (e.g., PyInstaller)
        return os.path.dirname(sys.executable)
    else:
        # If run as a normal .py script
        return os.path.dirname(os.path.abspath(sys.argv[0]))

def get_settings_path():
    """
    Finds a writable path for the settings file.
    1. Tries to find 'file_combiner_settings.json' in the script's directory.
    2. Tries to find 'file_combiner_settings.json' in the user's home directory.
    3. If not found, tries to *create* 'file_combiner_settings.json' in the script directory.
    4. If that fails (permissions), creates 'file_combiner_settings.json' in
       the user's home directory.
    """
    script_dir = get_script_directory()
    script_path = os.path.join(script_dir, "file_combiner_settings.json") 
    
    home_dir = os.path.expanduser("~")
    home_path = os.path.join(home_dir, "file_combiner_settings.json")

    # 1. Check if file exists in script dir
    if os.path.exists(script_path):
        return script_path
    
    # 2. Check if file exists in home dir
    if os.path.exists(home_path):
        return home_path

    # 3. If it exists nowhere, try to create it in the script dir
    try:
        with open(script_path, 'w') as f:
            json.dump(DEFAULT_FILTERS, f, indent=4)
        # Show a one-time message
        messagebox.showinfo(
            "Settings File Created",
            f"A new file_combiner_settings.json file has been created in the application's folder:\n\n{script_path}"
        )
        return script_path
    except Exception as e:
        print(f"Warning: Could not write to script directory: {e}")

    # 4. If script dir fails, fall back to home dir
    try:
        with open(home_path, 'w') as f:
            json.dump(DEFAULT_FILTERS, f, indent=4)
        # Show a one-time message
        messagebox.showwarning(
            "Settings File Created (Fallback)",
            "Could not write to the application's folder (check permissions).\n\n"
            f"A new settings file has been created in your home directory instead:\n\n{home_path}"
        )
        return home_path
    except Exception as e:
        # This is a critical failure
        messagebox.showerror(
            "Fatal Error",
            f"Could not create a settings file in the script or home directory.\n\nError: {e}"
        )
        return None

# --- Main Application ---

class FileCombinerApp:
    def __init__(self, root, settings_path):
        self.root = root
        self.root.title("File Combiner for AI")
        self.files = []
        self.settings_path = settings_path
        
        # Load filter settings from .json file
        self._load_settings()

        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(2, weight=1)

        self.main_frame = ttk.Frame(self.root, padding="10")
        self.main_frame.grid(row=2, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        self.main_frame.columnconfigure(0, weight=1)
        self.main_frame.rowconfigure(0, weight=1) 

        title_label = ttk.Label(self.root, text="Code Combiner for AI Analysis", font=("Helvetica", 16, "bold"))
        title_label.grid(row=0, column=0, pady=(10, 5), padx=10, sticky=tk.W)

        button_frame = ttk.Frame(self.root)
        button_frame.grid(row=1, column=0, padx=10, pady=5, sticky=(tk.W, tk.E))
        
        self.browse_files_button = ttk.Button(button_frame, text="Browse Files", command=self.browse_files)
        self.browse_files_button.pack(side=tk.LEFT, padx=(0, 5))

        self.browse_folder_button = ttk.Button(button_frame, text="Browse Folder", command=self.browse_folder)
        self.browse_folder_button.pack(side=tk.LEFT, padx=5)

        self.clear_button = ttk.Button(button_frame, text="Clear List", command=self.clear_list)
        self.clear_button.pack(side=tk.LEFT, padx=5)

        # --- Treeview (File List) ---
        self.tree = ttk.Treeview(self.main_frame, columns=("File", "Path"), show="headings")
        self.tree.heading("File", text="File Name")
        self.tree.heading("Path", text="File Path")
        self.tree.column("File", width=200)
        self.tree.column("Path", width=400)
        self.tree.grid(row=0, column=0, pady=5, sticky=(tk.W, tk.E, tk.N, tk.S))

        scrollbar = ttk.Scrollbar(self.main_frame, orient=tk.VERTICAL, command=self.tree.yview)
        self.tree.configure(yscroll=scrollbar.set)
        scrollbar.grid(row=0, column=1, sticky=(tk.N, tk.S))

        # --- Combine Button ---
        self.combine_button = ttk.Button(self.main_frame, text="Combine Files into One .txt", command=self.combine_files)
        self.combine_button.grid(row=1, column=0, columnspan=2, pady=10, sticky=(tk.W, tk.E))

    def _load_settings(self):
        """Loads filter settings from the .json file."""
        if not self.settings_path:
            # Failed to find or create a settings file
            settings_data = DEFAULT_FILTERS
        else:
            try:
                with open(self.settings_path, 'r') as f:
                    settings_data = json.load(f)
            except (FileNotFoundError, json.JSONDecodeError):
                settings_data = DEFAULT_FILTERS

        # Load settings, falling back to defaults for each key
        self.filter_mode = settings_data.get('mode', DEFAULT_FILTERS['mode'])
        
        self.filter_extensions_list = settings_data.get('extensions', DEFAULT_FILTERS['extensions'])
        self.ignore_files_list = settings_data.get('ignore_files', DEFAULT_FILTERS['ignore_files'])
        self.ignore_folders_list = settings_data.get('ignore_folders', DEFAULT_FILTERS['ignore_folders'])
        
        # Ensure lists are lowercase for comparison
        self.filter_extensions_list = [ext.lower() for ext in self.filter_extensions_list]
        self.ignore_files_list = [f.lower() for f in self.ignore_files_list]
        self.ignore_folders_list = [f.lower() for f in self.ignore_folders_list]


    def _is_file_allowed(self, file_path):
        """Checks if a file is allowed based on loaded settings."""
        file_name = os.path.basename(file_path).lower()

        # 1. Check against specific ignored filenames/suffixes
        #    *** THIS IS THE UPDATED LOGIC ***
        for ignore_pattern in self.ignore_files_list:
            if file_name.endswith(ignore_pattern):
                return False  # File matches an ignore pattern

        # 2. Check against extensions (Whitelist/Blacklist)
        mode = self.filter_mode
        
        if not self.filter_extensions_list:
            return True # No extension filter, allow all

        file_ext = os.path.splitext(file_path)[1].lower()
        if not file_ext: # Handle files with no extension
            return True if mode == "Blacklist" else False

        if mode == "Whitelist":
            return file_ext in self.filter_extensions_list
        elif mode == "Blacklist":
            return file_ext not in self.filter_extensions_list
        
        return True

    def browse_files(self):
        file_paths = filedialog.askopenfilenames()
        for file_path in file_paths:
            if self._is_file_allowed(file_path) and file_path not in self.files:
                self.files.append(file_path)
                file_name = os.path.basename(file_path)
                self.tree.insert("", tk.END, values=(file_name, file_path))

    def browse_folder(self):
        folder_path = filedialog.askdirectory()
        if not folder_path:
            return
        
        # We now use the list loaded from settings
        for dirpath, dirnames, filenames in os.walk(folder_path):
            # Modify dirnames *in place* to prevent os.walk from descending into them
            dirnames[:] = [d for d in dirnames if d.lower() not in self.ignore_folders_list]
            
            for file_name in filenames:
                file_path = os.path.join(dirpath, file_name)
                if self._is_file_allowed(file_path) and file_path not in self.files:
                    self.files.append(file_path)
                    self.tree.insert("", tk.END, values=(file_name, file_path))

    def clear_list(self):
        for item in self.tree.get_children():
            self.tree.delete(item)
        self.files.clear()

    def combine_files(self):
        if not self.files:
            messagebox.showwarning("No Files", "Please add files to the list before combining.")
            return

        output_file = filedialog.asksaveasfilename(
            defaultextension=".txt",
            filetypes=[("Text files", "*.txt"), ("All files", "*.*")]
        )
        if output_file:
            try:
                # Re-sort files by their path just before writing
                self.files.sort()
                
                with open(output_file, 'w', encoding='utf-8', errors='ignore') as outfile:
                    for file_path in self.files:
                        full_path = os.path.abspath(file_path).replace(os.sep, '/')
                        
                        outfile.write("-" * 80 + "\n")
                        outfile.write(f"// File: {full_path}\n")
                        outfile.write("-" * 80 + "\n")

                        file_content = None
                        encodings_to_try = ['utf-8', 'utf-8-sig', 'cp1252', 'latin-1']
                        
                        for encoding in encodings_to_try:
                            try:
                                with open(file_path, 'r', encoding=encoding) as infile:
                                    file_content = infile.read()
                                break
                            except (UnicodeDecodeError, TypeError, OSError):
                                continue
                        
                        if file_content is not None:
                            outfile.write(file_content)
                        else:
                            outfile.write(f"// Note: Could not decode file using common text encodings.\n")
                        
                        outfile.write("\n\n")
                messagebox.showinfo("Success", "Files combined successfully!")
            except Exception as e:
                messagebox.showerror("Error", f"Failed to combine files: {str(e)}")

if __name__ == "__main__":
    root = tk.Tk()
    
    # We must hide the root window *before* the message box
    # so the user isn't confused by a blank window.
    root.withdraw() 
    
    # Find or create the settings file.
    # This function will show its own pop-up messages.
    SETTINGS_PATH = get_settings_path()
    
    if SETTINGS_PATH is None:
        # A fatal error occurred, and the user was notified.
        root.destroy()
    else:
        # Now that settings are handled, show the main app
        root.deiconify() 
        app = FileCombinerApp(root, SETTINGS_PATH)
        root.mainloop()