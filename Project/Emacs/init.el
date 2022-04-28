;; -*- lexical-binding: t -*-

;; 增强文件内搜索和跳转函数定义
(package-install 'consult)
(global-set-key (kbd "C-s") 'consult-line)

;; modeline 完善
(package-install 'embark)
(global-set-key (kbd "C-;") 'embark-act)
(setq prefix-help-command 'embark-prefix-help-command)

;; modeline 里的内容加上注释
(package-install 'marginalia)
(marginalia-mode t)

;; modeline 是垂直
(package-install 'vertico)
(vertico-mode t)

;; 通过空格模糊搜索
(package-install 'orderless)
(setq completion-styles '(orderless))

;; 在 modeline 上显示按键和执行命令
(package-install 'keycast)
(keycast-mode t)

;;让鼠标滚动更好用
(setq mouse-wheel-scroll-amount '(1 ((shift) . 1) ((control) . nil)))
(setq mouse-wheel-progressive-speed nil)

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
(electric-pair-mode 1) ;; 括号补全

(setq mac-option-modifier 'meta
      mac-command-modifier 'super
      mac-right-option-modifier 'control
      )
(set-face-attribute 'default nil :height 120) ;; 字体设置为12pt
(tool-bar-mode -1) ;; 工具栏
(scroll-bar-mode -1) ;; 滚动栏
(global-linum-mode 1) ;; 行号
(setq-default cursor-type 'bar) ;; 更改
(icomplete-mode 1)

(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(package-selected-packages
   '(consult embark marginalia orderless vertico keycast org company)))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 )





