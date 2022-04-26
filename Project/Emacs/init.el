

;; f12 打开日记
(defun open-diary-file()
  (interactive)
  (find-file "~/Documents/EmacsFile/org/today.org"))
(global-set-key (kbd "<f12>") 'open-diary-file)

;; f1 打开配置文件
(defun open-init-file()
  (interactive)
  (find-file "~/.emacs.d/init.el"))
(global-set-key (kbd "<f2>") 'open-init-file) ;; 使用 F2 键来打开初始配置文件

(desktop-save-mode 1) ;; 保存当前窗口，下次启动恢复
(global-company-mode 1) ;; 补全
(setq company-idle-delay 0) ;; 补全加速

(setq mac-option-modifier 'meta
      mac-command-modifier 'super
      mac-right-option-modifier 'control
      )

(tool-bar-mode -1) ;; 工具栏
(scroll-bar-mode -1) ;; 滚动栏
(global-linum-mode 1) ;; 行号
(setq cursor-type 'bar) ;; 更改
(icomplete-mode 1)


(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(package-selected-packages '(org company)))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 )




