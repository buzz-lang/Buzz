(require 'newcomment)

;;
;; Buzz mode hook
;;
(defvar buzz-mode-hook nil
  "Function hook to execute when Buzz mode is run.")

;;
;; Customization group
;;
(defgroup buzz nil
  "Customization variables for Buzz mode."
  :tag "Buzz"
  :group 'languages)

;;
;; Indentation level
;;
(defcustom buzz-indent-level 2
  "Number of spaces for each indentation step in `buzz-mode'."
  :type 'integer
  :safe 'integerp
  :group 'buzz)

;;
;; Command to comment/uncomment text
;;
(defun buzz-comment-dwim (arg)
  "Comment or uncomment current line or region in a smart way.
For detail, see `comment-dwim'."
  (interactive "*P")
  (let ((comment-start "#") (comment-end ""))
    (comment-dwim arg)))

;;
;; Buzz syntax table
;;
(defvar buzz-syntax-table nil "Syntax table for `buzz-mode'.")
(setq buzz-syntax-table
      (let ((synTable (make-syntax-table)))
        (modify-syntax-entry ?# "< b" synTable)
        (modify-syntax-entry ?\n "> b" synTable)
        synTable))

;;
;; Define token classes and regular expressions
;;
(defconst buzz-identifier-regexp "[[:alnum:]_]+"
  "Regexp matching a Buzz identifier")
;; Keywords
(setq buzz-keywords '("var" "nil" "if" "else" "function" "return" "for" "while" "and" "or" "not"))
(setq buzz-keywords-regexp (regexp-opt buzz-keywords 'words))
(setq buzz-keywords nil)
;; Builtins
(setq buzz-builtins '("swarm" "stigmergy" "neighbors" "self" "id" "math"))
(setq buzz-builtins-regexp (regexp-opt buzz-builtins 'words))
(setq buzz-builtins nil)
;; Functions
(setq buzz-functions-regexp (concat "^\\s-*function\\s-+\\(" buzz-identifier-regexp "\\)"))

;;
;; Create font lock list
;;
(setq buzz-font-lock-list
      `(
        (,buzz-builtins-regexp   . font-lock-builtin-face)
        (,buzz-functions-regexp  1 font-lock-function-name-face)
        (,buzz-keywords-regexp   . font-lock-keyword-face)
        (,buzz-identifier-regexp . font-lock-variable-face)
        ))

;;
;; Indentation
;;
;; Marker regexp
(setq buzz-indent-marker-regexp (regexp-opt '("function" "if" "for" "while" "else")))
;; Indentation function
(defun buzz-indent-line ()
  "Indent current line as Buzz code."
  ;; Make it callable with M-x
  (interactive)
  ;; Move to the beginning of the line
  (beginning-of-line)
  ;; Save current position
  (setq cur-point (point))
  ;; Setup a variables and execute stuff
  ;; cur-indent is the current line indentation
  (let ((cur-indent 0) (stop nil))
    ;; Save current point in buffer and execute commands
    (save-excursion
      ;; Move backwards one line until you either
      ;; find an indentation marker,
      ;; or the first line in the buffer
      ;; Discard comment lines
      (while (and (not (bobp)) (not stop))
        (let ((comment (looking-at "^[[:blank:]]*#"))
              (marker (looking-at buzz-indent-marker-regexp)))
          (if (or comment (and (not comment) (not marker)))
              (forward-line -1)
            (setq stop 'true))))
      ;; +1 for each open parenthesis
      ;; -1 for each closed parenthesis and ;
      (setq cur-indent
            (- (count-matches "[[{(]"  (point) cur-point)
               (count-matches "[]})]" (point) cur-point))))
    ;; The line starts with a closed parenthesis, subtract 1
    (if (looking-at "^[[:blank:]]*[]})]")
        (setq cur-indent (- cur-indent 1)))
    ;; Make sure indentation is positive and
    ;; Multiply positive indentation by the width
    (if (< cur-indent 0)
        (setq cur-indent 0)
      (setq cur-indent (* cur-indent buzz-indent-level)))
    ;; Indent current line
    (indent-line-to cur-indent)))

;;
;; Define the mode
;;
(define-derived-mode buzz-mode prog-mode "Buzz"
  "Major mode for editing Buzz scripts."
  :group 'buzz
  :syntax-table buzz-syntax-table
  ;; Set font lock
  (setq font-lock-defaults '((buzz-font-lock-list)))
  ;; Modify the keymap
  (define-key buzz-mode-map [remap comment-dwim] 'buzz-comment-dwim)
  ;;
  (setq indent-line-function 'buzz-indent-line) 
  ;; Clear memory
  (setq buzz-keywords-regexp nil)
  (setq buzz-functions-regexp nil)
  )

;;
;; Associate Buzz mode with .bzz files
;;
(add-to-list 'auto-mode-alist '("\\.bzz\\'" . buzz-mode))

(provide 'buzz-mode)
