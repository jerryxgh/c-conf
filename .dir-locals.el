((nil . ((tab-width . 4)
         (whitespace-tab-width 4)
         (sentence-end-double-space . t)
         (fill-column . 80)
         ))
 (c-mode . ((c-file-style . "K&R")
            (indent-tabs-mode . nil)
            (c-basic-offset . 4)
            (flycheck-clang-include-path
             .
             ())))
 (objc-mode . ((c-file-style . "K&R")))
 ;; You must set bugtracker_debbugs_url in your bazaar.conf for this to work.
 ;; See admin/notes/bugtracker.
 (log-edit-mode . ((log-edit-rewrite-fixes
                    "[ \n](bug#\\([0-9]+\\))" . "debbugs:\\1")
                   (log-edit-font-lock-gnu-style . t)))
 (change-log-mode . ((add-log-time-zone-rule . t)
		     (fill-column . 74)
		     (bug-reference-url-format . "http://debbugs.gnu.org/%s")
		     (mode . bug-reference)))
 (diff-mode . ((mode . whitespace))))
