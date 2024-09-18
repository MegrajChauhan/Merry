dc _M_READ_       0o0 ;; open in read only mode
dc _M_WRITE_      0o1 ;; open in write only mode
dc _M_READ_WRITE_ 0o2 ;; open in read write mode
dc _M_APPEND_     0o2000  ;; open in append mode
dc _M_CREATE_     0o100 ;; Create the file if it doesn't exist(Shouldn't be used with fcntl)
dc _M_DIR_        0o2000000 ;; open a directory
dc _M_TMPFILE_    [0o200000000 | _M_DIR_] ;; open a temporary regular file
dc _M_TRUNC_      0o1000 ;; erase all data if opened in write mode and start from the beginning(Not to be used with fcntl)

;; Flags to pass with _M_CREATE_
dc _M_CREATE_RWX  0o700 ;; Create the file with read, write and execute permissions
dc _M_CREATE_R    0o400 ;; Create with just read permission
dc _M_CREATE_W    0o200 ;; Create with just write permission
dc _M_CREATE_X    0o100 ;; Create with just execute permission

;; The above flags suffice the User.
;; Flags for the group and others are not yet needed 

;; We will not include the flags such as O_ASYNC just yet
;; We need the need to arise.
;; The problem related to the O_CLOEXEC during multi-threaded execution is a
;; serious one. We will not include this flag just yet either.
