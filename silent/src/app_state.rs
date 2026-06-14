use std::fs;
use std::path::PathBuf;

use serde::{Deserialize, Serialize};

use crate::config::datadir;
#[cfg(test)]
use crate::config::set_datadir;

const DEFAULT_THEME: &str = "light";

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AppState {
    #[serde(default)]
    pub enabled_plugins: Vec<String>,
    #[serde(default = "default_active_theme")]
    pub active_theme: String,
}

fn default_active_theme() -> String {
    DEFAULT_THEME.to_string()
}

impl Default for AppState {
    fn default() -> Self {
        Self {
            enabled_plugins: Vec::new(),
            active_theme: default_active_theme(),
        }
    }
}

impl AppState {
    fn path() -> PathBuf {
        datadir().join("app.json")
    }

    pub fn load() -> Self {
        let path = Self::path();
        let content = match fs::read_to_string(&path) {
            Ok(content) => content,
            Err(_) => return Self::default(),
        };

        match serde_json::from_str::<Self>(&content) {
            Ok(mut state) => {
                state.dedup_plugins();
                if state.active_theme.is_empty() {
                    state.active_theme = default_active_theme();
                }
                state
            }
            Err(e) => {
                log::error!("AppState::load() failed to parse {}: {e}", path.display());
                Self::default()
            }
        }
    }

    pub fn save(&self) {
        let path = Self::path();
        if let Some(parent) = path.parent() {
            let _ = fs::create_dir_all(parent);
        }

        let mut state = self.clone();
        state.dedup_plugins();

        match serde_json::to_string_pretty(&state) {
            Ok(content) => {
                if let Err(e) = fs::write(&path, content) {
                    log::error!("AppState::save() failed to write {}: {e}", path.display());
                }
            }
            Err(e) => log::error!("AppState::save() failed to serialize: {e}"),
        }
    }

    fn dedup_plugins(&mut self) {
        let mut unique = Vec::with_capacity(self.enabled_plugins.len());
        for plugin_id in self.enabled_plugins.drain(..) {
            if !unique.contains(&plugin_id) {
                unique.push(plugin_id);
            }
        }
        self.enabled_plugins = unique;
    }
}

pub fn app_enabled_plugins() -> Vec<String> {
    AppState::load().enabled_plugins
}

pub fn app_set_plugin_enabled(id: String, enabled: bool) {
    let mut state = AppState::load();

    if enabled {
        if !state.enabled_plugins.contains(&id) {
            state.enabled_plugins.push(id);
        }
    } else {
        state.enabled_plugins.retain(|plugin_id| plugin_id != &id);
    }

    state.save();
}

pub fn app_active_theme() -> String {
    AppState::load().active_theme
}

pub fn app_set_active_theme(name: String) {
    let mut state = AppState::load();
    state.active_theme = if name.is_empty() {
        default_active_theme()
    } else {
        name
    };
    state.save();
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::Mutex;
    use std::time::{SystemTime, UNIX_EPOCH};

    static TEST_LOCK: Mutex<()> = Mutex::new(());
    const TEST_SUBDIR: &str = "silent-app-state-tests";

    fn with_isolated_datadir(test: impl FnOnce(PathBuf)) {
        let _guard = TEST_LOCK.lock().expect("test lock poisoned");
        let nonce = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .expect("clock before unix epoch")
            .as_nanos();
        let dir =
            std::env::temp_dir().join(format!("{TEST_SUBDIR}-{}-{nonce}", std::process::id()));
        let _ = fs::remove_dir_all(&dir);
        set_datadir(dir.clone());
        test(dir.clone());
        let _ = fs::remove_dir_all(&dir);
    }

    #[test]
    fn load_missing_file_returns_defaults() {
        with_isolated_datadir(|base| {
            let _ = fs::remove_file(base.join("app.json"));

            let state = AppState::load();

            assert!(state.enabled_plugins.is_empty());
            assert_eq!(state.active_theme, "light");
        });
    }

    #[test]
    fn roundtrip_persists_enabled_plugins_and_theme() {
        with_isolated_datadir(|base| {
            let _ = fs::remove_file(base.join("app.json"));

            app_set_plugin_enabled("plugin-a".to_string(), true);
            app_set_plugin_enabled("plugin-a".to_string(), true);
            app_set_plugin_enabled("plugin-b".to_string(), true);
            app_set_plugin_enabled("plugin-a".to_string(), false);
            app_set_active_theme("solarized".to_string());

            let state = AppState::load();

            assert_eq!(state.enabled_plugins, vec!["plugin-b".to_string()]);
            assert_eq!(state.active_theme, "solarized");
        });
    }
}
